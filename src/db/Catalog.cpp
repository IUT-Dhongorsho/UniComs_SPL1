#include "Catalog.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <filesystem>

void Catalog::load(const std::string &ddlFile)
{
    schemas.clear();
    std::cout << ddlFile << "\n";
    namespace fs = std::filesystem;

    // If file does not exist → create empty one
    if (!fs::exists(ddlFile))
    {
        std::ofstream create(ddlFile);
        if (!create)
            throw std::runtime_error("Failed to create ddl file");
        create.close();
        return; // empty catalog
    }

    // Otherwise load normally
    std::ifstream file(ddlFile);
    if (!file)
        throw std::runtime_error("Failed to open ddl file");

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;

        while (std::getline(ss, segment, ','))
            parts.push_back(segment);

        // Expected format:
        // table,field,type,primary(0/1),unique(0/1)
        if (parts.size() < 5)
            continue;

        std::string tableName = parts[0];

        Field field;
        field.name = parts[1];
        field.type = parts[2];
        field.primary = (parts[3] == "1");
        field.unique = (parts[4] == "1");

        if (schemas.find(tableName) == schemas.end())
        {
            Schema schema;
            schema.name = tableName;
            schemas[tableName] = schema;
        }

        schemas[tableName].fields.push_back(field);
    }
}

const Schema &Catalog::getSchema(const std::string &table) const
{
    if (schemas.find(table) == schemas.end())
        throw std::runtime_error("Schema not found for table: " + table);

    return schemas.at(table);
}

bool Catalog::hasTable(const std::string &table) const
{
    return schemas.find(table) != schemas.end();
}

const std::unordered_map<std::string, Schema> &Catalog::getAll() const
{
    return schemas;
}

// ---------------------- addSchema ----------------------
void Catalog::addSchema(const Schema &schema, const std::string &ddlFile)
{
    if (hasTable(schema.name))
        throw std::runtime_error("Schema already exists in catalog: " + schema.name);

    // Add to in-memory map
    schemas.emplace(schema.name, schema);

    // Append to ddl.csv
    std::ofstream out(ddlFile, std::ios::app);
    if (!out)
        throw std::runtime_error("Failed to open ddl file for writing: " + ddlFile);

    for (size_t i = 0; i < schema.fields.size(); ++i)
    {
        const Field &f = schema.fields[i];
        out << schema.name << ","
            << f.name << ","
            << f.type << ","
            << (f.primary ? "1" : "0") << ","
            << (f.unique ? "1" : "0")
            << "\n";
    }

    std::cout << "Schema for table '" << schema.name << "' added to catalog.\n";
}
