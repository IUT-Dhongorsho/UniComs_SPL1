#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <iostream>

#include "Catalog.hpp"
#include "Table_Engine.hpp"
#include "Exceptions.hpp"

class Database
{
private:
    Catalog catalog;
    std::unordered_map<std::string, TableEngine> engines;
    bool initialized = false;

public:
    Database() = default;

    // Initialize database by loading ddl and building table engines
    void init(const std::string &ddlFile = "src/db/ddl.csv")
    {
        catalog.load(ddlFile);

        // Create table engines for all schemas
        for (const auto &pair : catalog.getAll())
        {
            const std::string &tableName = pair.first;
            const Schema &schema = pair.second;

            engines.emplace(tableName, TableEngine(schema));
            engines.at(tableName).load();
        }

        initialized = true;
    }

    // ================= TEMPLATE API (Facade) =================

    template <typename T>
    void create()
    {
        ensureInitialized();

        Schema schema = T::schema();
        const std::string tableName = schema.name;

        if (catalog.hasTable(tableName))
            throw std::runtime_error("Table already exists: " + tableName);

        // Add schema to catalog (updates ddl.csv)
        catalog.addSchema(schema);

        // Create a new TableEngine for this table
        TableEngine engine(schema);
        engine.saveEmpty(); // creates the CSV with header

        // Insert engine into engines map
        engines.emplace(tableName, std::move(engine));

        std::cout << "Table '" << tableName << "' created successfully.\n";
    }

    template <typename T>
    void insert(const T &obj)
    {
        ensureInitialized();

        std::string tableName = T::schema().name;

        if (!catalog.hasTable(tableName))
            throw TableNotFound("Table not found: " + tableName);

        auto &engine = engines.at(tableName);
        engine.insertRow(obj.serialize());
    }

    template <typename T>
    std::optional<T> query(const std::string &field,
                           const std::string &value)
    {
        ensureInitialized();

        std::string tableName = T::schema().name;

        if (!catalog.hasTable(tableName))
            throw TableNotFound("Table not found: " + tableName);

        auto &engine = engines.at(tableName);
        auto row = engine.findByField(field, value);

        if (!row.has_value())
            return std::nullopt;

        return T::deserialize(row.value());
    }

    template <typename T>
    void remove(const std::string &primaryKey)
    {
        ensureInitialized();

        std::string tableName = T::schema().name;

        if (!catalog.hasTable(tableName))
            throw TableNotFound("Table not found: " + tableName);

        auto &engine = engines.at(tableName);
        engine.deleteByPrimary(primaryKey);
    }

    template <typename T>
    void update(const std::string &primaryKey, const T &newObj)
    {
        ensureInitialized();

        std::string tableName = T::schema().name;

        if (!catalog.hasTable(tableName))
            throw TableNotFound("Table not found: " + tableName);

        auto &engine = engines.at(tableName);
        engine.updateByPrimary(primaryKey, newObj.serialize());
    }

private:
    void ensureInitialized() const
    {
        if (!initialized)
            throw std::runtime_error("Database not initialized. Call init() first.");
    }
};
