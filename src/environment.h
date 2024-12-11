#pragma once

#include "expr.h"
#include <unordered_map>

struct Environment {
    void define(std::string_view name, Value value);
    RuntimeError get(Token* token, Value& in_value);

    std::unordered_map<std::string_view, Value> values;
};