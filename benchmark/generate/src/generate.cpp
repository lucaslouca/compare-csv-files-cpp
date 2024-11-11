#include <iostream>
#include <fstream>
#include <string>
#include <random>

using namespace std;

std::string generate_random_string(const int min_len, const int max_len)
{
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string tmp;
    std::random_device rd;                                   // obtain a random number generator for hardware
    std::mt19937 gen(rd());                                  // seed the generator
    std::uniform_int_distribution<> distr(min_len, max_len); // define the range
    const int len = distr(gen);                              // generate random string length

    tmp.reserve(len);
    for (int i = 0; i < len; ++i)
    {
        tmp += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return tmp;
}

int main(int argc, char **argv)
{
    unsigned int rows = 100;
    unsigned int cols = 5;
    ofstream fs;
    std::string fname = "benchmark/data/file2.csv";
    fs.open(fname.c_str());

    for (unsigned int j = 0; j < cols; ++j)
    {
        fs << "COLUMN" << j << ",";
    }
    fs << endl;

    for (unsigned int i = 0; i < rows; ++i)
    {
        for (unsigned int j = 0; j < cols; ++j)
        {
            fs << "R" << i << "C" << j << "-" << generate_random_string(5, 15) << ",";
        }
        fs << endl;
    }

    fs.close();
    return 0;
}