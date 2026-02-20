#pragma once

#include <unordered_map>
#include <optional>
#include "Schema.hpp"

class IndexManager
{
private:
    std::unordered_map<std::string, size_t> primaryIndex;
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, size_t>>
        uniqueIndexes;

public:
    void build(const Schema &schema,
               const std::vector<std::vector<std::string>> &rows);

    bool primaryExists(const std::string &key) const;

    bool uniqueExists(const std::string &field,
                      const std::string &value) const;

    std::optional<size_t> findByPrimary(const std::string &key) const;

    std::optional<size_t> findByUnique(const std::string &field,
                                       const std::string &value) const;

    void clear();
};
