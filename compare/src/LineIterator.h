#ifndef LINEITERATOR_H
#define LINEITERATOR_H
#include <string>
#include <iostream>

class LineIterator
{
public:
    typedef std::input_iterator_tag iterator_category;
    typedef std::string value_type;
    typedef std::size_t difference_type;
    typedef std::string *pointer;
    typedef std::string &reference;

    LineIterator(std::istream &str);
    LineIterator();

    // Pre Increment
    LineIterator &operator++();

    // Post Increment
    LineIterator operator++(int);

    std::string const &operator*() const;
    std::string const *operator->() const;

    bool operator==(LineIterator const &rhs) const;
    bool operator!=(LineIterator const &rhs) const;

private:
    std::istream *m_str;
    std::string m_line;
};

#endif