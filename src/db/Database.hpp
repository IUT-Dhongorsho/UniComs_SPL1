#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_set>
#include "csv.hpp"

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

public:
    void loadTablesfromDdl()
    {
        std::ifstream ddlFile("ddl.csv");
        std::string line;
        std::unordered_set<std::string> newTables;
        while (std::getline(ddlFile, line))
        {
            // std::cout << line << "\n";
            std::string first_word;
            std::stringstream ss(line);
            char deli = ',';
            std::getline(ss, first_word, deli);
            // std::cout << first_word << "\n";
            if (first_word == "table")
                continue;
            newTables.insert(first_word);
        }
        tables = newTables;
        for (std::string s : newTables)
        {
            std::cout << "Table: " << s << std::endl;
        }
    }
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
        loadTablesfromDdl();
        /* ---------- Prevent duplicate table ---------- */
        if (tables.count(tableName))
        {
            std::cerr << "Table already exists\n";
            return;
        }

        const std::string ddlFileName = "ddl.csv";
        const std::string ddlHeader =
            "table,field,position,type,nullable,default,primary_key,unique";

        bool ddlExists = false;
        bool headerValid = false;

        /* ---------- Check ddl.csv ---------- */
        {
            std::ifstream ddlIn(ddlFileName);
            if (ddlIn.good())
            {
                ddlExists = true;

                std::string headerLine;
                std::getline(ddlIn, headerLine);
                headerValid = (headerLine == ddlHeader);
            }
        }

        /* ---------- Create or fix ddl.csv ---------- */
        if (!ddlExists || !headerValid)
        {
            std::ofstream ddlOut(ddlFileName, std::ios::trunc);
            if (!ddlOut)
            {
                std::cerr << "Failed to create ddl.csv\n";
                return;
            }
            ddlOut << ddlHeader << "\n";
        }

        /* ---------- Append schema ---------- */
        std::ofstream ddlAppend(ddlFileName, std::ios::app);
        if (!ddlAppend)
        {
            std::cerr << "Failed to open ddl.csv for appending\n";
            return;
        }

        for (size_t i = 0; i < schema.fields.size(); ++i)
        {
            const Field &f = schema.fields[i];
            std::vector<std::string> toParse = {schema.name, f.name, std::to_string(i), fieldTypeToString(f.type), "false", "true", "false"};
            std::string csvLine = toCSV(toParse);
            // std::cout << csvLine << std::endl;
            ddlAppend << csvLine;
        }

        /* ---------- Create table file ---------- */
        std::ofstream tableFile(tableName + ".csv", std::ios::app);
        if (!tableFile)
        {
            std::cerr << "Failed to create table file\n";
            return;
        }

        tables.insert(tableName);

        std::cout << "Table '" << tableName << "' created successfully\n";
    }
    /* -------- INSERT -------- */

    template <typename T>
    void insert(const T &data)
    {
        Schema schema = T::schema();
        std::vector<std::string> serializedData = data.serialize();
        // Load schema from ddl for constraint check. Currently it's hardcoded.
        // Unique key Validation
        std::ifstream schemaFileRead(schema.name + ".csv");
        std::string tableLine;
        while (std::getline(schemaFileRead, tableLine))
        {
            std::stringstream ss(tableLine);
            std::string id;
            std::getline(ss, id, ',');
            if (serializedData[0] == id)
            {
                std::cerr << "Unique constraint violation. Insert failed!\n";
                return;
            }
            // if (schema.name == "User")
            // {

            // }
        }
        std::ofstream schemaFileWrite(schema.name + ".csv", std::ios::app);
        schemaFileWrite << toCSV(serializedData);
    }
};

#endif // DATABASE_H
