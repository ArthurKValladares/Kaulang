#pragma once

#include "lib/string.h"
#include "lib/array.h"

#include "expr.h"

#include <unordered_map>
#include <functional>

using CallableCallback = std::function<Value(Array<Value>, KauCompiler*, Arena*, Environment*)>;
struct Callable {
    Callable() {}
    Callable(int arity, CallableCallback callback) 
        : m_arity(arity)
        , m_callback(callback)
    {}

    int m_arity = 0;
    CallableCallback m_callback;
};

// TODO: A lot of these refs should maybe be ptrs?
// TODO: these should all just return ptrs and i handle the error later
struct Environment {
    void define(const Token* token, Value value);
    void define(String str, Value value);
    bool contains(const Token* token) const;
    RuntimeError set(const Token* token, Value value);
    RuntimeError get(const Token* token, Value& in_value);
    Value get_unchecked(String name);
    RuntimeError get_at(const Token* token, u64 distance, Value& in_value);

    void define_callable(const Token* token, Callable in_callable);
    void define_callable(const String &str, Callable in_callable);
    RuntimeError get_callable(const Token* token, Callable& in_callable);
    Callable* get_callable(String name);

    void define_class(const Token* token, Class in_class);
    void define_class(const String &str, Class in_class);
    RuntimeError get_class(const Token* token, Class** in_class);

    std::unordered_map<String, Value, StringHasher> values;
    std::unordered_map<String, Callable, StringHasher> callables;
    std::unordered_map<String, Class, StringHasher> classes;

    Environment* ancestor(u64 distance);
    
    Environment* enclosing = nullptr;
};