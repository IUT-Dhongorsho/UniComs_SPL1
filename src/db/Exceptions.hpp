#pragma once
#include <stdexcept>
#include <string>

class ConstraintViolation : public std::runtime_error
{
public:
    explicit ConstraintViolation(const std::string &msg)
        : std::runtime_error("Constraint violation: " + msg) {}
};

class TableNotFound : public std::runtime_error
{
public:
    explicit TableNotFound(const std::string &table)
        : std::runtime_error("Table not found: " + table) {}
};

class FieldNotFound : public std::runtime_error
{
public:
    explicit FieldNotFound(const std::string &field)
        : std::runtime_error("Field not found: " + field) {}
};
