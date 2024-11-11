#ifndef MATCHRESULT_H
#define MATCHRESULT_H

#include <string>
#include <map>
#include <set>

using ResultSet = std::map<std::string, std::map<std::string, std::set<int>>>;

struct match_result
{
    std::string val;
    std::map<std::string, std::set<int>> fname_col;
};

#endif