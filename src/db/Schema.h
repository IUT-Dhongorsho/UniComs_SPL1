#pragma once
#include <string>
#include <vector>
#include <stdexcept>

struct Field
{
    std::string name; // field name
    std::string type; // "string", "integer", "boolean", etc.
    bool primary = false;
    bool unique = false;
};

struct Schema
{
    std::string name;
    std::vector<Field> fields;

    // Get index of a field in fields vector
    int getFieldIndex(const std::string &field) const
    {
        for (size_t i = 0; i < fields.size(); ++i)
        {
            if (fields[i].name == field)
                return static_cast<int>(i);
        }
        return -1;
    }

    // Returns primary key field name (empty if none)
    std::string getPrimaryField() const
    {
        for (const auto &f : fields)
        {
            if (f.primary)
                return f.name;
        }
        return "";
    }

    // Returns all unique fields except primary key
    std::vector<std::string> getUniqueFields() const
    {
        std::vector<std::string> result;
        for (const auto &f : fields)
        {
            if (f.unique && !f.primary)
                result.push_back(f.name);
        }
        return result;
    }
};
