#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

/* ---------------- Schema Types ---------------- */

enum class FieldType
{
    String,
    Integer,
    Boolean
};

inline std::string fieldTypeToString(FieldType type)
{
    switch (type)
    {
    case FieldType::String:
        return "string";
    case FieldType::Integer:
        return "integer";
    case FieldType::Boolean:
        return "boolean";
    default:
        return "unknown";
    }
}

struct Field
{
    std::string name;
    FieldType type;
};

struct Schema
{
    std::string name;
    std::vector<Field> fields;
};

/* ---------------- Database ---------------- */

class Database
{
private:
    // Tracks created tables
    std::unordered_set<std::string> tables;

    // In-memory index: table → row positions
    std::unordered_map<std::string, std::vector<std::streampos>> rowIndex;

public:
    /* -------- CREATE -------- */

    template <typename T>
    void create(const std::string &tableName)
    {
        Schema schema = T::schema();

        if (schema.name != tableName)
        {
            std::cerr << "Schema name does not match table name\n";
            return;
        }

        // Append schema to ddl.json
        std::ofstream ddlFile("ddl.json", std::ios::app);
        if (!ddlFile)
        {
            std::cerr << "Failed to open ddl.json\n";
            return;
        }

        ddlFile << "{"
                << "\"table\":\"" << tableName << "\","
                << "\"fields\":[";

        for (size_t i = 0; i < schema.fields.size(); ++i)
        {
            const Field &f = schema.fields[i];
            ddlFile << "{"
                    << "\"name\":\"" << f.name << "\","
                    << "\"type\":\"" << fieldTypeToString(f.type) << "\""
                    << "}";

            if (i + 1 < schema.fields.size())
                ddlFile << ",";
        }

        ddlFile << "]}\n";
        ddlFile.close();

        // Create table file
        std::ofstream tableFile(tableName + ".json", std::ios::app);
        if (!tableFile)
        {
            std::cerr << "Failed to create table file\n";
            return;
        }

        tableFile.close();

        tables.insert(tableName);
        rowIndex[tableName]; // initialize empty index

        std::cout << "Table '" << tableName << "' created successfully\n";
    }

    /* -------- INSERT -------- */

    template <typename T>
    void insert(const T &data)
    {
        Schema schema = T::schema();
        const std::string &tableName = schema.name;

        // 1️⃣ Check table existence
        if (tables.find(tableName) == tables.end())
        {
            std::cerr << "Table '" << tableName << "' does not exist\n";
            return;
        }

        // 2️⃣ Open table file
        std::ofstream tableFile(tableName + ".json", std::ios::app);
        if (!tableFile)
        {
            std::cerr << "Failed to open table file\n";
            return;
        }

        // 3️⃣ Record file offset for indexing
        std::streampos pos = tableFile.tellp();

        // 4️⃣ Write NDJSON row
        tableFile << "{";
        tableFile << data.name;
        tableFile << "},\n";

        std::cout << data.name;

        tableFile.close();

        // 5️⃣ Update in-memory index
        rowIndex[tableName].push_back(pos);

        std::cout << "Inserted row into '" << tableName << "'\n";
    }
};

#endif // DATABASE_H
