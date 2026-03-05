#include "Table_Engine.hpp"
#include <fstream>
#include <algorithm>

TableEngine::TableEngine(const Schema &schema)
    : schema(schema), storage("src/db/" + schema.name + ".csv") {}

void TableEngine::load()
{
    auto allRows = storage.loadAll();

    // Skip header row if present (first column matches first field name)
    size_t startIdx = 0;
    if (!allRows.empty() && !schema.fields.empty() &&
        !allRows[0].empty() && allRows[0][0] == schema.fields[0].name)
        startIdx = 1;

    rows = std::vector<std::vector<std::string>>(allRows.begin() + startIdx, allRows.end());
    indexManager.build(schema, rows);
}

void TableEngine::insertRow(const std::vector<std::string> &row)
{
    if (row.size() != schema.fields.size())
        throw ConstraintViolation("Column count mismatch");

    // Primary key check
    std::string primaryField = schema.getPrimaryField();
    int primaryCol = schema.getFieldIndex(primaryField);
    if (primaryCol >= 0 && indexManager.primaryExists(row[primaryCol]))
        throw ConstraintViolation("Duplicate primary key: " + row[primaryCol]);

    // Unique field checks
    for (const auto &uniqueField : schema.getUniqueFields())
    {
        int col = schema.getFieldIndex(uniqueField);
        if (col >= 0 && indexManager.uniqueExists(uniqueField, row[col]))
            throw ConstraintViolation("Duplicate unique field: " + uniqueField);
    }

    rows.push_back(row);
    storage.appendRow(row);
    indexManager.build(schema, rows);
}

std::optional<std::vector<std::string>> TableEngine::findByField(const std::string &field, const std::string &value)
{
    int col = schema.getFieldIndex(field);
    if (col < 0)
        throw FieldNotFound(field);

    if (field == schema.getPrimaryField())
    {
        auto idx = indexManager.findByPrimary(value);
        if (!idx)
            return std::nullopt;
        return rows[*idx];
    }

    auto idx = indexManager.findByUnique(field, value);
    if (idx)
        return rows[*idx];

    for (const auto &r : rows)
        if (static_cast<size_t>(col) < r.size() && r[col] == value)
            return r;

    return std::nullopt;
}

std::vector<std::vector<std::string>> TableEngine::findAllByField(const std::string &field, const std::string &value)
{
    int col = schema.getFieldIndex(field);
    if (col < 0)
        throw FieldNotFound(field);

    std::vector<std::vector<std::string>> result;
    for (const auto &r : rows)
        if (static_cast<size_t>(col) < r.size() && r[col] == value)
            result.push_back(r);
    return result;
}

std::vector<std::vector<std::string>> TableEngine::getAll()
{
    return rows;
}

void TableEngine::deleteByPrimary(const std::string &key)
{
    auto idx = indexManager.findByPrimary(key);
    if (!idx)
        return;
    rows.erase(rows.begin() + *idx);
    storage.rewriteAll(rows);
    indexManager.build(schema, rows);
}

void TableEngine::deleteAllByField(const std::string &field, const std::string &value)
{
    int col = schema.getFieldIndex(field);
    if (col < 0)
        throw FieldNotFound(field);

    rows.erase(
        std::remove_if(rows.begin(), rows.end(), [&](const std::vector<std::string> &r) {
            return static_cast<size_t>(col) < r.size() && r[col] == value;
        }),
        rows.end());

    storage.rewriteAll(rows);
    indexManager.build(schema, rows);
}

void TableEngine::updateByPrimary(const std::string &key, const std::vector<std::string> &newRow)
{
    auto idx = indexManager.findByPrimary(key);
    if (!idx)
        throw ConstraintViolation("Primary key not found: " + key);
    rows[*idx] = newRow;
    storage.rewriteAll(rows);
    indexManager.build(schema, rows);
}

void TableEngine::saveEmpty()
{
    std::ofstream out(storage.getFilename(), std::ios::trunc);
    if (!out)
        throw std::runtime_error("Failed to create table file: " + storage.getFilename());

    for (size_t i = 0; i < schema.fields.size(); ++i)
    {
        out << schema.fields[i].name;
        if (i + 1 != schema.fields.size())
            out << ",";
    }
    out << "\n";
}