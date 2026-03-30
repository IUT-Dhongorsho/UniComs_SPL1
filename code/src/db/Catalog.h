#pragma once

#include <unordered_map>
#include <string>
#include "Schema.h"

class Catalog
{
private:
    std::unordered_map<std::string, Schema> schemas;

public:
    void load(const std::string &ddlFile);

    const Schema &getSchema(const std::string &table) const;

    bool hasTable(const std::string &table) const;

    const std::unordered_map<std::string, Schema> &getAll() const;

    // Add schema to catalog and update ddl.csv
    void addSchema(const Schema &schema, const std::string &ddlFile = "src/db/ddl.csv");
};
