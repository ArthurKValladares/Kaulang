#pragma once

#include "expr.h"
#include <unordered_map>
#include <vector>
#include <functional>

// TODO: temp struct
using CallableCallback = std::function<Value(std::vector<Expr*> const&)>;
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
    void define_callable(const std::string &str, Callable in_callable);
    RuntimeError get_callable(const Token* token, Callable& in_callable);

    // TODO: Annoying that im using an allocated string here
    std::unordered_map<std::string, Value> values;
    std::unordered_map<std::string, Callable> callables;

    Environment* enclosing = nullptr;
};