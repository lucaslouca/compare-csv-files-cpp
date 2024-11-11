
#include "AbstractComparator.h"

AbstractComparator::AbstractComparator(std::vector<std::string> _fnames, std::vector<std::string> _column_indices) : fnames(std::move(_fnames)), column_indices(std::move(_column_indices))
{
}

ResultSet AbstractComparator::result() const
{
    return result_set;
}

AbstractComparator::~AbstractComparator()
{
}