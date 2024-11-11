#ifndef COLUMNRANGE_H
#define COLUMNRANGE_H
#include "LineIterator.h"
#include <istream>

class ColumnRange
{
public:
    ColumnRange(std::istream &str);
    LineIterator begin() const;
    LineIterator end() const;

private:
    std::istream &stream;
};

#endif