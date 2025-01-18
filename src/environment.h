#pragma once

#include "lib/string.h"

#include "expr.h"

#include <unordered_map>
#include <vector>
#include <functional>

using CallableCallback = std::function<Value(std::vector<Value> const&, KauCompiler*, Arena*, Environment*)>;
struct Callable {
    Callable() {}
    Callable(int arity, CallableCallback callback) 
        : m_arity(arity)
        , m_callback(callback)
    {}

    int m_arity = 0;
    CallableCallback m_callback;
};

struct Environment {
    void define(const Token* token, Value value);
    bool contains(const Token* token) const;
    RuntimeError set(const Token* token, Value value);
    RuntimeError get(const Token* token, Value& in_value);

    void define_callable(const Token* token, Callable in_callable);
    void define_callable(const String &str, Callable in_callable);
    RuntimeError get_callable(const Token* token, Callable& in_callable);

    std::unordered_map<String, Value, StringHasher> values;
    std::unordered_map<String, Callable, StringHasher> callables;

    Environment* enclosing = nullptr;
};