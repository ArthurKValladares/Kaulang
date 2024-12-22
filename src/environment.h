#pragma once

#include "expr.h"
#include <unordered_map>

struct Environment {
    void define(const Token* token, Value value);
    bool contains(const Token* token) const;
    RuntimeError set(const Token* token, Value value);
    RuntimeError get(const Token* token, Value& in_value);

    // TODO: Annoying that im using an allocated string here
    std::unordered_map<std::string, Value> values;
    Environment* enclosing = nullptr;
};