#ifndef ABSTRACTCOMPARATOR_H
#define ABSTRACTCOMPARATOR_H

#include <vector>
#include "Matchresult.h"

class AbstractComparator
{
public:
    AbstractComparator(std::vector<std::string> _fnames, std::vector<std::string> _column_indices);
    virtual ~AbstractComparator();
    virtual bool compare() = 0;
    ResultSet result() const;

protected:
    std::vector<std::string> fnames;
    std::vector<std::string> column_indices;
    ResultSet result_set;
};

#endif