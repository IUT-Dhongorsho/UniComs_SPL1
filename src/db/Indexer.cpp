#include "Indexer.hpp"

void IndexManager::build(const Schema &schema,
                         const std::vector<std::vector<std::string>> &rows)
{
    clear();

    int primaryIndexCol = schema.getFieldIndex(schema.getPrimaryField());
    auto uniqueFields = schema.getUniqueFields();

    for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
    {
        const auto &row = rows[rowIndex];

        if (primaryIndexCol >= 0)
        {
            primaryIndex[row[primaryIndexCol]] = rowIndex;
        }

        for (const auto &field : uniqueFields)
        {
            int col = schema.getFieldIndex(field);
            uniqueIndexes[field][row[col]] = rowIndex;
        }
    }
}

bool IndexManager::primaryExists(const std::string &key) const
{
    return primaryIndex.contains(key);
}

bool IndexManager::uniqueExists(const std::string &field,
                                const std::string &value) const
{
    if (!uniqueIndexes.contains(field))
        return false;

    return uniqueIndexes.at(field).contains(value);
}

std::optional<size_t> IndexManager::findByPrimary(const std::string &key) const
{
    if (!primaryIndex.contains(key))
        return std::nullopt;

    return primaryIndex.at(key);
}

std::optional<size_t> IndexManager::findByUnique(const std::string &field,
                                                 const std::string &value) const
{
    if (!uniqueIndexes.contains(field))
        return std::nullopt;

    if (!uniqueIndexes.at(field).contains(value))
        return std::nullopt;

    return uniqueIndexes.at(field).at(value);
}

void IndexManager::clear()
{
    primaryIndex.clear();
    uniqueIndexes.clear();
}
