#pragma once
#include <unordered_map>
#include <optional>
#include <vector>
#include <stdexcept>
#include "Catalog.hpp"
#include "Table_Engine.hpp"
#include "Exceptions.hpp"

class Database
{
private:
    Catalog catalog;
    std::unordered_map<std::string, TableEngine> engines;
    bool initialized = false;

    void ensureInitialized() const
    {
        if (!initialized)
            throw std::runtime_error("Database not initialized. Call init() first.");
    }

public:
    Database() = default;

    void init(const std::string &ddlFile = "src/db/ddl.csv")
    {
        catalog.load(ddlFile);
        for (const auto &pair : catalog.getAll())
        {
            engines.emplace(pair.first, TableEngine(pair.second));
            engines.at(pair.first).load();
        }
        initialized = true;
    }

    // Create table if it does not exist yet
    template <typename T>
    void create()
    {
        ensureInitialized();
        Schema schema = T::schema();
        if (catalog.hasTable(schema.name))
            return;
        catalog.addSchema(schema);
        TableEngine engine(schema);
        engine.saveEmpty();
        engines.emplace(schema.name, std::move(engine));
    }

    template <typename T>
    void insert(const T &obj)
    {
        ensureInitialized();
        std::string name = T::schema().name;
        if (!catalog.hasTable(name))
            throw TableNotFound(name);
        engines.at(name).insertRow(obj.serialize());
    }

    template <typename T>
    std::optional<T> query(const std::string &field, const std::string &value)
    {
        ensureInitialized();
        std::string name = T::schema().name;
        if (!catalog.hasTable(name))
            throw TableNotFound(name);
        auto row = engines.at(name).findByField(field, value);
        if (!row)
            return std::nullopt;
        return T::deserialize(*row);
    }

    template <typename T>
    std::vector<T> queryAll(const std::string &tableName, const std::string &field, const std::string &value)
    {
        std::vector<T> result;
        auto rows = engines.at(tableName).findAllByField(field, value);
        if (!rows.empty()) {
            for (const auto &row : rows)
                result.push_back(T::deserialize(row));
        }
        return result;
    }

    template <typename T>
    std::vector<T> getAll(const std::string &tableName)
    {
        std::vector<T> result;
        auto rows = engines.at(tableName).getAll();
        if (!rows.empty()) {
            for (const auto &row : rows)
                result.push_back(T::deserialize(row));
        }
        return result;
    }

    template <typename T>
    void remove(const std::string &primaryKey)
    {
        ensureInitialized();
        std::string name = T::schema().name;
        if (!catalog.hasTable(name))
            throw TableNotFound(name);
        engines.at(name).deleteByPrimary(primaryKey);
    }

    template <typename T>
    void removeAllByField(const std::string &field, const std::string &value)
    {
        ensureInitialized();
        std::string name = T::schema().name;
        if (!catalog.hasTable(name))
            throw TableNotFound(name);
        engines.at(name).deleteAllByField(field, value);
    }

    template <typename T>
    void update(const std::string &primaryKey, const T &newObj)
    {
        ensureInitialized();
        std::string name = T::schema().name;
        if (!catalog.hasTable(name))
            throw TableNotFound(name);
        engines.at(name).updateByPrimary(primaryKey, newObj.serialize());
    }

    void delete_by_field(const std::string &tableName, const std::string &field, const std::string &value)
    {
        engines.at(tableName).deleteAllByField(field, value);
    }
};