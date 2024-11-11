#ifndef CSVROW_H
#define CSVROW_H
#include <string_view>
#include <cstddef>
#include <istream>
#include <string>
#include <vector>

class CSVRow
{
public:
    std::string_view operator[](std::size_t index) const;
    std::size_t size() const;
    void next(std::istream &str);

private:
    std::string m_line;
    std::vector<int> m_data;

    // The non-member function operator>> will have access to CSVRow's private members
    friend std::istream &operator>>(std::istream &str, CSVRow &data);
};

#endif