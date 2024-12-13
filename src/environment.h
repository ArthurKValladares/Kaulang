#pragma once

#include "expr.h"
#include <unordered_map>

struct Environment {
    void define(Token* token, Value value);
    bool contains(Token* token) const;
    RuntimeError get(Token* token, Value& in_value);

    // TODO: Annoying that im using an allocated string here
    std::unordered_map<std::string, Value> values;
};