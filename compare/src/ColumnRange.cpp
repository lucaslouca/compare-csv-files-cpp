#include "ColumnRange.h"

ColumnRange::ColumnRange(std::istream &str) : stream(str)
{
}
LineIterator ColumnRange::begin() const
{
    return LineIterator{stream};
}

LineIterator ColumnRange::end() const
{
    return LineIterator{};
}