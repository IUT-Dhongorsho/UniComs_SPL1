#pragma once

#include <vector>
#include <optional>
#include <string>
#include "Schema.h"
#include "CsvStorage.h"
#include "Indexer.h"
#include "Exceptions.h"

class TableEngine
{
private:
    Schema schema;
    CSVStorage storage;
    IndexManager indexManager;
    std::vector<std::vector<std::string>> rows;

public:
    explicit TableEngine(const Schema &schema);

    // Load table data from CSV and build indexes
    void load();

    // Insert a row with primary/unique checks
    void insertRow(const std::vector<std::string> &row);

    // Find a row by any field
    std::optional<std::vector<std::string>>
    findByField(const std::string &field,
                const std::string &value);

    std::vector<std::vector<std::string>> findAllByField(const std::string &field, const std::string &value);
    std::vector<std::vector<std::string>> getAll();
    void deleteAllByField(const std::string &field, const std::string &value);

    // Delete a row by primary key
    void deleteByPrimary(const std::string &key);

    // Update a row by primary key
    void updateByPrimary(const std::string &key,
                         const std::vector<std::string> &newRow);

    // Initialize empty CSV file with header
    void saveEmpty();
};
