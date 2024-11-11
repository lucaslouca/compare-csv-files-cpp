#ifndef CSVRANGE_H
#define CSVRANGE_H
#include "CSVIterator.h"
#include <istream>

class CSVRange
{
public:
    CSVRange(std::istream &str);
    CSVIterator begin() const;
    CSVIterator end() const;

private:
    std::istream &stream;
};

#endif