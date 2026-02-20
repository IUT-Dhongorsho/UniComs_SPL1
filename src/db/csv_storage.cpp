#include "csv_storage.hpp"
#include <fstream>
#include <sstream>

CSVStorage::CSVStorage(const std::string &file)
    : filename(file)
{
}
const std::string &CSVStorage::getFilename() const
{
    return CSVStorage::filename;
}
std::vector<std::vector<std::string>> CSVStorage::loadAll()
{
    std::vector<std::vector<std::string>> rows;

    std::ifstream file(filename);
    if (!file)
        return rows;

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> row;

        while (std::getline(ss, segment, ','))
            row.push_back(segment);

        rows.push_back(row);
    }

    return rows;
}

void CSVStorage::appendRow(const std::vector<std::string> &row)
{
    std::ofstream file(filename, std::ios::app);
    for (size_t i = 0; i < row.size(); ++i)
    {
        file << row[i];
        if (i + 1 < row.size())
            file << ",";
    }
    file << "\n";
}

void CSVStorage::rewriteAll(const std::vector<std::vector<std::string>> &rows)
{
    std::ofstream file(filename, std::ios::trunc);

    for (const auto &row : rows)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            file << row[i];
            if (i + 1 < row.size())
                file << ",";
        }
        file << "\n";
    }
}
