#ifndef DISKBASEDCOMPARATOR_H
#define DISKBASEDCOMPARATOR_H

#include <map>
#include <set>
#include <string>
#include "AbstractComparator.h"

class DiskBasedComparator : public AbstractComparator
{
    using Filename_To_Pairs_Linecount_TmpFilename = std::map<std::string, std::vector<std::pair<int, std::string>>>;
    using Filename_To_Linecount = std::map<std::string, std::vector<unsigned long>>;

public:
    using AbstractComparator::AbstractComparator;
    bool compare() override;
    ~DiskBasedComparator();

private:
    void delete_file(const std::string fname) const;
    void show_progress(const double &show_perc, std::string &bar, std::size_t bar_len, const unsigned long &total_match_count) const;
    std::string extract_column(const char *fname, int col, int (DiskBasedComparator::*call_back)(const char *, const char *, std::set<std::string> &, int)); // call_back is actually a pointer to a member variable of DiskBasedComparator
    int column_val_callback(const char *begin, const char *end, std::set<std::string> &col_data, int col);
    std::map<std::string, std::vector<unsigned long>> compute_line_count(const Filename_To_Pairs_Linecount_TmpFilename &col_fnames) const;
    unsigned long long compute_total_comparissons(const DiskBasedComparator::Filename_To_Linecount &line_count) const;
    std::string file_with_min_count(const std::map<std::string, std::vector<unsigned long>> &line_count) const;
    Filename_To_Pairs_Linecount_TmpFilename extract_columns(const std::map<std::string, std::set<int>> &fnames_cols);
    void process_result(match_result &cur_result);
    std::map<std::string, std::set<int>> fnames_cols;

    // for holding tmp files: "fname.csv" -> [(<line count>, "fname.csv<col>_tmp"),...]
    Filename_To_Pairs_Linecount_TmpFilename col_fnames;
};

#endif