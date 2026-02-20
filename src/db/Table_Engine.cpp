#include "Table_Engine.hpp"
#include <fstream>
#include <iostream>

TableEngine::TableEngine(const Schema &schema)
    : schema(schema),
      storage("src/db/" + schema.name + ".csv") // CSV file name based on table
{
}

// Load all rows and rebuild indexes
void TableEngine::load()
{
    rows = storage.loadAll();
    indexManager.build(schema, rows);
}

// Insert row with constraint enforcement
void TableEngine::insertRow(const std::vector<std::string> &row)
{
    if (row.size() != schema.fields.size())
        throw ConstraintViolation("Column count mismatch");

    // Primary key check
    std::string primaryField = schema.getPrimaryField();
    int primaryCol = schema.getFieldIndex(primaryField);
    if (primaryCol >= 0)
    {
        std::string key = row[primaryCol];
        if (indexManager.primaryExists(key))
            throw ConstraintViolation("Duplicate primary key: " + key);
    }

    // Unique field check
    for (const auto &uniqueField : schema.getUniqueFields())
    {
        int col = schema.getFieldIndex(uniqueField);
        if (indexManager.uniqueExists(uniqueField, row[col]))
            throw ConstraintViolation("Duplicate unique field: " + uniqueField);
    }

    // Append row
    rows.push_back(row);
    storage.appendRow(row);

    // Rebuild indexes
    indexManager.build(schema, rows);
}

// Find a row by a field (primary, unique, or any)
std::optional<std::vector<std::string>>
TableEngine::findByField(const std::string &field,
                         const std::string &value)
{
    int col = schema.getFieldIndex(field);
    if (col < 0)
        throw FieldNotFound(field);

    // Primary key lookup
    if (field == schema.getPrimaryField())
    {
        auto idx = indexManager.findByPrimary(value);
        if (!idx.has_value())
            return std::nullopt;

        return rows[idx.value()];
    }

    // Unique field lookup
    if (indexManager.findByUnique(field, value).has_value())
    {
        size_t idx = indexManager.findByUnique(field, value).value();
        return rows[idx];
    }

    // Linear search
    for (const auto &r : rows)
    {
        if (r[col] == value)
            return r;
    }

    return std::nullopt;
}

// Delete row by primary key
void TableEngine::deleteByPrimary(const std::string &key)
{
    auto idx = indexManager.findByPrimary(key);
    if (!idx.has_value())
        return;

    rows.erase(rows.begin() + idx.value());
    storage.rewriteAll(rows);
    indexManager.build(schema, rows);
}

// Update row by primary key
void TableEngine::updateByPrimary(const std::string &key,
                                  const std::vector<std::string> &newRow)
{
    auto idx = indexManager.findByPrimary(key);
    if (!idx.has_value())
        throw ConstraintViolation("Primary key not found: " + key);

    rows[idx.value()] = newRow;
    storage.rewriteAll(rows);
    indexManager.build(schema, rows);
}

// Create empty CSV file with header
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
