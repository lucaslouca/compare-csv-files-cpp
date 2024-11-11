#ifndef UTILITIES_H
#define UTILITIES_H

#include <set>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>

namespace Utilities
{
    inline std::set<int> split(const std::string &str, char sep)
    {
        std::set<int> result;
        std::stringstream ss(str);
        for (int i; ss >> i;)
        {
            result.emplace(i);
            if (ss.peek() == sep)
            {
                ss.ignore();
            }
        }
        return result;
    }

    inline unsigned long count_lines(const std::string fname)
    {
        unsigned long count = 0;
        std::ifstream file(fname);
        std::string line;
        while (std::getline(file, line))
        {
            ++count;
        }
        return count;
    }

    template <typename T>
    inline std::string to_string(const std::set<T> &s)
    {
        std::stringstream result;
        std::copy(s.begin(), s.end(), std::ostream_iterator<T>(result, " "));
        return result.str();
    }

    template <typename TargetType>
    inline TargetType convert(const std::string &value)
    {
        TargetType converted;
        std::istringstream stream(value);
        stream >> converted;
        return converted;
    }

    template <typename K, typename V>
    inline std::vector<K> keys(std::map<K, V> m)
    {
        std::vector<K> keys;
        for (const auto &[k, v] : m)
        {
            keys.push_back(k);
        }

        return keys;
    }
}

#endif