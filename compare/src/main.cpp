#include <fstream>
#include <iostream>
#include <vector>

#include <algorithm>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "CSVRange.h"
#include "ArgParser.h"
#include "Matchresult.h"
#include "Utilities.h"
#include "DiskBasedComparator.h"
#include <sstream>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <chrono>

// using stream buffer + mmap()
#include <streambuf>
#include <istream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    /* C POSIX libary file control options */
#include <unistd.h>   /* C POSIX libary system calls: open, close */
#include <sys/mman.h> /* memory management declarations: mmap, munmap */
#include <errno.h>    /* Std C libary system error numbers: errno */
#include <err.h>      /* GNU C lib error messages: err */

// for benchmarking
#include <benchmark/benchmark.h>

// using stream buffer + mmap()
struct membuf : std::streambuf
{
    membuf(char *start, std::size_t size)
    {
        this->setg(start, start, start + size);
    }
};

int read_lines_membuf(const char *fname)
{
    int fd = open(fname, O_RDONLY);
    struct stat fs;
    char *start;

    if (fd == -1)
    {
        err(1, "open membuf: %s", fname);
        return 0;
    }

    if (fstat(fd, &fs) == -1)
    {
        err(1, "stat:%s", fname);
        return 0;
    }

    /* fs.st_size could have been 0 actually */
    start = reinterpret_cast<char *>(mmap(0, fs.st_size, PROT_READ, MAP_SHARED, fd, 0));
    if (start == (void *)-1)
    {
        err(1, "mmap: %s", fname);
        close(fd);
        return 0;
    }

    membuf sbuf(start, fs.st_size);
    std::istream in(&sbuf);

    unsigned int count = 0;
    for (std::string line; std::getline(in, line);)
    {
        ++count;
        // std::cout << line << std::endl;
    }

    munmap(start, fs.st_size);
    close(fd);
    return count;
}

// Using CSVRange
unsigned int read_lines_csvrange(const char *fname)
{
    unsigned int count = 0;
    std::ifstream file(fname);
    for (auto _ : CSVRange(file))
    {
        ++count;
        // std::cout << "1st Element(" << row[0] << ")\n";
    }

    return count;
}

static void print_usage(const std::string &name)
{
    std::cout << "usage: " << name << " [-h] [-f <file>] [-c <column index>] [<args>]"
              << "\n"
              << "Compare:\n"
              << "  -f    file to compare\n"
              << "  -c    comma separated list of column indices (zero-based) to compare. Use -1 for consider all columns.\n"
              << "  -m    load entire files into memory\n"
              << "Miscellaneous:\n"
              << "  -h    display this help text and exit\n"
              << "Example:\n"
              << "  " << name << " -f ../../benchmark/data/file1.csv -c 0 -f ../../benchmark/data/file2.csv -c 1,2\n"
              << "  compares columns 0, 5 and 6 (either) of file1.csv with column 2 of file2.csv for an exact match."
              << std::endl;
}

void print_result(ResultSet result_set)
{
    std::cout << "\nResults:\n";
    if (result_set.size() == 0)
    {
        std::cout << "No matches found." << std::endl;
    }
    else
    {
        for (const auto &[val, fname_columns] : result_set)
        {
            std::cout << "Value '" << val << "' in:\n";
            for (const auto &[fname, columns] : fname_columns)
            {
                std::cout << fname << " Columns: " << Utilities::to_string(columns) << std::endl;
            }
            std::cout << "\n";
        }
    }
}

int main(int argc, char **argv)
{
    ArgParser args(argc, argv);
    if (args.has_option("-h"))
    {
        print_usage(argv[0]);
        return 0;
    }

    if (!args.has_option("-f") || !args.has_option("-c"))
    {
        print_usage(argv[0]);
        return 1;
    }

    std::vector<std::string> fnames = args.option("-f");
    if (fnames.size() < 2)
    {
        err(1, "Please provide at least 2 files for comparisson.");
    }

    std::vector<std::string> column_indices = args.option("-c");
    if (column_indices.size() != fnames.size())
    {
        err(1, "Please provide at least %lu column indices for comparisson.", fnames.size());
    }

    // Listen for sigint event line Ctrl^c
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    std::atomic<bool> shutdown_requested = false;
    std::mutex cv_mutex;
    std::condition_variable cv;

    auto signal_handler = [&shutdown_requested, &cv, &sigset]()
    {
        int signum = 0;
        // wait untl a signal is delivered
        sigwait(&sigset, &signum);
        shutdown_requested.store(true);

        // notify all waiting workers to check their predicate
        cv.notify_all();
        return signum;
    };

    auto ft_signal_handler = std::async(std::launch::async, signal_handler);

    auto worker = [&shutdown_requested, &cv_mutex, &cv, &fnames, &column_indices]()
    {
        std::unique_ptr<AbstractComparator> comparator = std::make_unique<DiskBasedComparator>(fnames, column_indices);
        comparator->compare();
        auto const &result = comparator->result();
        print_result(result);

        while (shutdown_requested.load() == false)
        {
            std::unique_lock lock(cv_mutex);
            // wait for up to an hour
            cv.wait_for(lock, std::chrono::seconds(1),
                        // when the condition variable is woken up and this predicate returns true, the wait is stopped
                        [&shutdown_requested]()
                        {
                            bool stop_waiting = shutdown_requested.load();
                            return stop_waiting; });
        }

        return shutdown_requested.load();
    };

    // spawn some workers
    std::vector<std::future<bool>> workers;
    for (int i = 0; i < 1; ++i)
    {
        workers.push_back(std::async(std::launch::async, worker));
    }

    std::cout << "Waiting for SIGTERM or SIGINT ([CTRL]+[c])...\n";

    int signal = ft_signal_handler.get();
    std::cout << "Received signal " << signal << "\n";

    // wait for workers
    for (auto &future : workers)
    {
        std::cout << "Worker observed shutdown request: " << std::boolalpha << future.get() << "\n";
    }

    std::cout << "Clean shutdown\n";

    return 0;
}