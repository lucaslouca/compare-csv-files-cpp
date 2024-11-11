#include "CSVRange.h"

CSVRange::CSVRange(std::istream &str) : stream(str)
{
}
CSVIterator CSVRange::begin() const
{
    return CSVIterator{stream};
}

CSVIterator CSVRange::end() const
{
    return CSVIterator{};
}
