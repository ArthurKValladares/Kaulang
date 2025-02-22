#pragma once

#include "lib/string.h"
#include "lib/array.h"

#include "expr.h"

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

struct Environment {
    void init(Arena* arena);
    
    void define(Arena* arena, const String str, Value in_value);
    bool contains(const String name) const;
    bool set(const String name, Value value);
    Value* get(const String name);
    Value* get_at(const String name, u64 distance);

    void define_callable(Arena* arena, const String str, Callable in_callable);
    Callable* get_callable(const String name);

    void define_class(Arena* arena, const String str, Class in_class);
    Class* get_class(const String name);

    Map values;
    Map callables;
    Map classes;

    Environment* ancestor(u64 distance);
    
    Environment* enclosing = nullptr;
};