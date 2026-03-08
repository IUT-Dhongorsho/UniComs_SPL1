#include "csv_storage.hpp"
#include <fstream>
#include <sstream>

CSVStorage::CSVStorage(const std::string &file)
    : filename(file)
{
}

const std::string &CSVStorage::getFilename() const
{
    return filename;
}

// Parse one CSV line respecting double-quoted fields.
// A field may be wrapped in "..." to contain commas or quotes (RFC 4180).
static std::vector<std::string> parseLine(const std::string &line)
{
    std::vector<std::string> row;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (inQuotes)
        {
            if (c == '"')
            {
                // Peek ahead — two consecutive quotes → literal quote
                if (i + 1 < line.size() && line[i + 1] == '"')
                {
                    field += '"';
                    ++i;
                }
                else
                {
                    inQuotes = false;
                }
            }
            else
            {
                field += c;
            }
        }
        else
        {
            if (c == '"')
            {
                inQuotes = true;
            }
            else if (c == ',')
            {
                row.push_back(field);
                field.clear();
            }
            else
            {
                field += c;
            }
        }
    }

    row.push_back(field); // last field
    return row;
}

// Serialize one field: wrap in quotes if it contains a comma or a quote.
static std::string quoteField(const std::string &s)
{
    bool needsQuoting = s.find(',') != std::string::npos ||
                        s.find('"') != std::string::npos ||
                        s.find('\n') != std::string::npos;

    if (!needsQuoting)
        return s;

    std::string out = "\"";
    for (char c : s)
    {
        if (c == '"') out += '"'; // escape by doubling
        out += c;
    }
    out += '"';
    return out;
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
        if (line.empty())
            continue;
        rows.push_back(parseLine(line));
    }

    return rows;
}

void CSVStorage::appendRow(const std::vector<std::string> &row)
{
    std::ofstream file(filename, std::ios::app);
    for (size_t i = 0; i < row.size(); ++i)
    {
        file << quoteField(row[i]);
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
            file << quoteField(row[i]);
            if (i + 1 < row.size())
                file << ",";
        }
        file << "\n";
    }
}