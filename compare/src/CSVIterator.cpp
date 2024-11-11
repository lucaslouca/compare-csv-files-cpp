#include "CSVIterator.h"

CSVIterator::CSVIterator(std::istream &str) : m_str(str.good() ? &str : nullptr)
{
    ++(*this);
}

CSVIterator::CSVIterator() : m_str(nullptr) {}

// Pre Increment
CSVIterator &CSVIterator::operator++()
{
    if (m_str)
    {
        if (!((*m_str) >> m_row))
        {
            m_str = nullptr;
        }
    }
    return *this;
}

// Post Increment
CSVIterator CSVIterator::operator++(int)
{
    CSVIterator tmp(*this);
    ++(*this);
    return tmp;
}

CSVRow const &CSVIterator::operator*() const
{
    return m_row;
}
CSVRow const *CSVIterator::operator->() const
{
    return &m_row;
}

bool CSVIterator::operator==(CSVIterator const &rhs) const
{
    return ((this == &rhs) || ((this->m_str == nullptr) && (rhs.m_str == nullptr)));
}

bool CSVIterator::operator!=(CSVIterator const &rhs) const
{
    return !((*this) == rhs);
}
