#ifndef CSVITERATOR_H
#define CSVITERATOR_H
#include "CSVRow.h"
#include <iostream>

class CSVIterator
{
public:
    typedef std::input_iterator_tag iterator_category;
    typedef CSVRow value_type;
    typedef std::size_t difference_type;
    typedef CSVRow *pointer;
    typedef CSVRow &reference;

    CSVIterator(std::istream &str);
    CSVIterator();

    // Pre Increment
    CSVIterator &operator++();

    // Post Increment
    CSVIterator operator++(int);

    CSVRow const &operator*() const;
    CSVRow const *operator->() const;

    bool operator==(CSVIterator const &rhs) const;
    bool operator!=(CSVIterator const &rhs) const;

private:
    std::istream *m_str;
    CSVRow m_row;
};

#endif