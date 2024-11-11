#include "DiskBasedComparator.h"
#include "Utilities.h"
#include "ColumnRange.h"

#include <cmath>
#include <filesystem>
#include <limits.h> // ULONG_MAX
#include <istream>
#include <iostream>
#include <fstream>
#include <iterator>
#include <chrono>

// mmap()
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    /* C POSIX libary file control options */
#include <unistd.h>   /* C POSIX libary system calls: open, close */
#include <sys/mman.h> /* memory management declarations: mmap, munmap */
#include <errno.h>    /* Std C libary system error numbers: errno */
#include <err.h>      /* GNU C lib error messages: err */

std::string DiskBasedComparator::extract_column(const char *fname, int col, int (DiskBasedComparator::*call_back)(const char *, const char *, std::set<std::string> &, int))
{
    std::set<std::string> col_data;
    int fd = open(fname, O_RDONLY);
    struct stat fs;
    char *buf, *buf_end;
    char *begin, *end, c;

    if (fd == -1)
    {
        err(1, "open mmap: %s", fname);
        return {};
    }

    if (fstat(fd, &fs) == -1)
    {
        err(1, "stat:%s", fname);
        return {};
    }

    /* fs.st_size could have been 0 actually */
    buf = reinterpret_cast<char *>(mmap(0, fs.st_size, PROT_READ, MAP_SHARED, fd, 0));
    if (buf == (void *)-1)
    {
        err(1, "mmap: %s", fname);
        close(fd);
        return {};
    }

    std::string col_fname = std::string(fname) + std::to_string(col) + "_tmp";
    std::ofstream output_file(col_fname);
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");

    buf_end = buf + fs.st_size;
    begin = end = buf;
    while (1)
    {
        if (!(*end == '\r' || *end == '\n'))
        {
            if (++end < buf_end)
            {
                continue;
            }
        }
        else if (1 + end < buf_end)
        {
            /* see if we got "\r\n" or "\n\r" here */
            c = *(1 + end);
            if ((c == '\r' || c == '\n') && c != *end)
            {
                ++end;
            }
        }

        /* call the call back and check error indication. Announce error
        here, because we didn't tell call_back the file name */
        if (!(this->*call_back)(begin, end, col_data, col))
        {
            err(1, "[callback] %s", fname);
            break;
        }

        if ((begin = ++end) >= buf_end)
        {
            break;
        }

        if (col_data.size() == 2)
        {
            // Flush to disk
            std::copy(col_data.begin(), col_data.end(), output_iterator);
            col_data.clear();
        }
    }
    std::copy(col_data.begin(), col_data.end(), output_iterator);

    munmap(buf, fs.st_size);
    close(fd);
    return col_fname;
}

int DiskBasedComparator::column_val_callback(const char *begin, const char *end, std::set<std::string> &col_data, int col)
{
    std::string line(begin, end - begin + 1);
    int current_col = 0;
    std::string::size_type last_pos = 0;
    std::string::size_type pos = 0;
    while ((pos = line.find(',', pos)) != std::string::npos)
    {
        if (current_col == col)
        {
            col_data.emplace(line.substr(last_pos, pos - last_pos));
        }
        ++pos;
        last_pos = pos;
        ++current_col;
    }
    return 1;
}

DiskBasedComparator::Filename_To_Pairs_Linecount_TmpFilename DiskBasedComparator::extract_columns(const std::map<std::string, std::set<int>> &fnames_cols)
{
    DiskBasedComparator::Filename_To_Pairs_Linecount_TmpFilename col_fnames;
    for (const auto &[fname, columns] : fnames_cols)
    {
        if (col_fnames.find(fname) == col_fnames.end())
        {
            col_fnames.emplace(std::make_pair(fname, std::vector<std::pair<int, std::string>>()));
        }

        for (auto &col : columns)
        {
            std::string col_fname = extract_column(fname.c_str(), col, &DiskBasedComparator::column_val_callback);
            if (col_fname.empty())
            {
                err(1, "Column %d for %s appears to be empty.", col, fname.c_str());
            }

            // For each file to compare store the number of lines of its shortest column
            col_fnames[fname].emplace_back(std::make_pair(col, col_fname));
        }
    }
    return col_fnames;
}

DiskBasedComparator::Filename_To_Linecount DiskBasedComparator::compute_line_count(const DiskBasedComparator::Filename_To_Pairs_Linecount_TmpFilename &col_fnames) const
{
    // For each CSV keep track of the number of rows in each column
    DiskBasedComparator::Filename_To_Linecount line_count;
    for (const auto &[fname, columns] : col_fnames)
    {
        for (auto &col_cfname : columns)
        {
            unsigned long count = Utilities::count_lines(col_cfname.second);
            if (line_count.find(fname) == line_count.end())
            {
                line_count.emplace(std::make_pair(fname, std::vector<unsigned long>()));
            }
            line_count[fname].push_back(count);
        }
    }

    return line_count;
}

std::string DiskBasedComparator::file_with_min_count(const DiskBasedComparator::Filename_To_Linecount &line_count) const
{
    unsigned long min_line_count = ULONG_MAX;
    std::string fname_seek;

    for (const auto &[fname, columns] : line_count)
    {
        for (auto &count : columns)
        {
            if (count < min_line_count)
            {
                min_line_count = count;
                fname_seek = fname;
            }
        }
    }

    return fname_seek;
}

unsigned long long DiskBasedComparator::compute_total_comparissons(const DiskBasedComparator::Filename_To_Linecount &line_count) const
{
    unsigned long long total_comparissons = 0;
    std::vector<std::string> tmp_fnames = Utilities::keys(line_count);
    for (int i = 0; i < tmp_fnames.size(); ++i)
    {
        std::string fname = tmp_fnames[i];
        std::vector<unsigned long> col_line_count = line_count.at(fname);

        for (const unsigned long &count1 : col_line_count)
        {
            for (int j = i + 1; j < tmp_fnames.size(); ++j)
            {
                std::string fname2 = tmp_fnames[j];
                std::vector<unsigned long> col_line_count2 = line_count.at(fname2);

                if (fname.compare(fname2) != 0)
                {
                    for (const unsigned long &count2 : col_line_count2)
                    {
                        total_comparissons += count1 * count2;
                    }
                }
            }
        }
    }

    return total_comparissons;
}

bool DiskBasedComparator::compare()
{
    for (int i = 0; i < fnames.size(); ++i)
    {
        fnames_cols.emplace(std::make_pair(fnames[i], Utilities::split(column_indices[i], ',')));
    }

    std::cout << "Comparing...\n";
    col_fnames = extract_columns(fnames_cols);
    DiskBasedComparator::Filename_To_Linecount line_count = compute_line_count(col_fnames);
    std::string fname_seek = file_with_min_count(line_count);
    unsigned long long total_comparissons = compute_total_comparissons(line_count);

    if (total_comparissons == 0)
    {
        std::cout << "Nothing to compare. Bye!\n";
    }
    else
    {
        std::cout << "Status:\n";
        unsigned long total_match_count = 0;
        std::size_t bar_len = std::min<unsigned long>(30, total_comparissons);
        unsigned long progress_chunk = total_comparissons / bar_len;
        unsigned long current_count = 0;
        std::string bar;

        // We start with the file that has the least rows
        for (auto &col_fname : col_fnames[fname_seek])
        {
            std::ifstream file_seek(col_fname.second);
            for (auto val_seek : ColumnRange(file_seek))
            {
                // If we have n files to compare, each seek value needs to appear in exactly n-1 other files
                int required_match_count = fnames.size() - 1;

                match_result cur_result;
                cur_result.val = val_seek;
                cur_result.fname_col.emplace(std::make_pair(fname_seek, std::set<int>()));
                cur_result.fname_col[fname_seek].emplace(col_fname.first);

                // Iterate over the rest of the CSV files
                for (const auto &[fname_cur, col_fnames_cur] : col_fnames)
                {
                    // Make sure we are not comparing with self
                    if (fname_seek.compare(fname_cur) != 0)
                    {
                        // Starting a new CSV file. Set match to false.
                        bool match = false;

                        // Iterate over the columns
                        for (const auto &col_fname_cur : col_fnames_cur)
                        {
                            std::ifstream file_cur(col_fname_cur.second);
                            for (auto val_cur : ColumnRange(file_cur))
                            {
                                if (val_seek.compare(val_cur) == 0)
                                {
                                    match = true;
                                    if (cur_result.fname_col.find(fname_cur) == cur_result.fname_col.end())
                                    {
                                        cur_result.fname_col.emplace(std::make_pair(fname_cur, std::set<int>()));
                                    }
                                    cur_result.fname_col[fname_cur].emplace(col_fname_cur.first);
                                }

                                ++current_count;
                                if (current_count % progress_chunk == 0 || current_count == total_comparissons)
                                {
                                    long perc = current_count / (total_comparissons * 1.0) * 100.0;
                                    show_progress(perc, bar, bar_len, total_match_count);
                                }
                            }
                        }

                        if (match)
                        {
                            // We had a match  in one of the columns of the current file.
                            --required_match_count;
                        }
                        else
                        {
                            // If we cannot find a match in any one the the CSV files, then there is no need to continue with the rest
                            break;
                        }
                    }
                }

                if (required_match_count == 0)
                {
                    process_result(cur_result);
                    ++total_match_count;
                }
            }
        }
    }

    return true;
}

void DiskBasedComparator::process_result(match_result &cur_result)
{
    if (result_set.find(cur_result.val) == result_set.end())
    {
        result_set.emplace(std::make_pair(cur_result.val, cur_result.fname_col));
    }
    else
    {
        for (const auto &[tmp_fname, columns] : cur_result.fname_col)
        {
            if (result_set[cur_result.val].find(tmp_fname) == result_set[cur_result.val].end())
            {
                result_set[cur_result.val].emplace(std::make_pair(tmp_fname, std::set<int>()));
            }
            result_set[cur_result.val][tmp_fname].insert(cur_result.fname_col[tmp_fname].begin(), cur_result.fname_col[tmp_fname].end());
        }
    }
}

void DiskBasedComparator::show_progress(const double &show_perc, std::string &bar, std::size_t bar_len, const unsigned long &total_match_count) const
{
    if (bar.size() <= bar_len)
    {
        std::string empty(bar_len - bar.size(), '.');
        std::cout << "\r[" << bar << empty << "]"
                  << "[" << std::ceil(show_perc) << "]"
                  << "[" << total_match_count << " matches"
                  << "]\r" << std::flush;
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        bar.insert(0, 1, '#');
    }
}

void DiskBasedComparator::delete_file(const std::string fname) const
{
    try
    {
        if (!std::filesystem::remove(fname))
        {
            std::cout << "File " << fname << " not found" << std::endl;
        }
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        std::cout << "Filesystem error: " << err.what() << "\n";
    }
}

DiskBasedComparator::~DiskBasedComparator()
{
    // Clean up tmp files
    for (const auto &[fname, col_name_pairs] : col_fnames)
    {
        for (const auto &p : col_name_pairs)
        {
            delete_file(p.second);
        }
    }
}