#include "LineIterator.h"
#include <string>

LineIterator::LineIterator(std::istream &str) : m_str(str.good() ? &str : nullptr)
{
    ++(*this);
}

LineIterator::LineIterator() : m_str(nullptr) {}

// Pre Increment
LineIterator &LineIterator::operator++()
{
    if (m_str)
    {
        if (!std::getline((*m_str), m_line, '\n'))
        {
            m_str = nullptr;
        }
    }
    return *this;
}

// Post Increment
LineIterator LineIterator::operator++(int)
{
    LineIterator tmp(*this);
    ++(*this);
    return tmp;
}

std::string const &LineIterator::operator*() const
{
    return m_line;
}
std::string const *LineIterator::operator->() const
{
    return &m_line;
}

bool LineIterator::operator==(LineIterator const &rhs) const
{
    return ((this == &rhs) || ((this->m_str == nullptr) && (rhs.m_str == nullptr)));
}

bool LineIterator::operator!=(LineIterator const &rhs) const
{
    return !((*this) == rhs);
}