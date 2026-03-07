#pragma once

#include <string>
#include <vector>

class CSVStorage
{
private:
    std::string filename;

public:
    explicit CSVStorage(const std::string &file);
    const std::string &getFilename() const;
    std::vector<std::vector<std::string>> loadAll();

    void appendRow(const std::vector<std::string> &row);

    void rewriteAll(const std::vector<std::vector<std::string>> &rows);
};
