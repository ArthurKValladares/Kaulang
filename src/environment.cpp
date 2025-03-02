#include "environment.h"

void Environment::init(Arena* arena) {
    values.allocate(arena);
    callables.allocate(arena);
    classes.allocate(arena);
}

void Environment::define(Arena* arena, const String str, Value in_value) {
    values.insert(arena, (void*) &str, sizeof(String), HASH_STR(str), &in_value, sizeof(Value));
}

bool Environment::contains(const String name) const {
    const Value* val = (const Value*) values.get_const(HASH_STR(name));
    if(val != nullptr) {
        return true;
    } else {
        if (enclosing != nullptr) {
            return enclosing->contains(name);
        }

        return false;
    }
}

bool Environment::set(const String name, Value value) {
    Value* val = (Value*) values.get(HASH_STR(name));
    if (val != nullptr) {
        *val = value;
        return true;
    } else {
        if (enclosing != nullptr) {
            return enclosing->set(name, value);
        }

        return false;
    }
}

Value* Environment::get(String name) {
    Value* val = (Value*) values.get(HASH_STR(name));
    if (val != nullptr) {
        return val;
    } else {
        if (enclosing != nullptr) {
            return enclosing->get(name);
        }
        return nullptr;
    }
}

Environment* Environment::ancestor(u64 distance) {
    Environment* env = this;
    for (u64 i = 0; i < distance; ++i) {
        env = env->enclosing;
    }
    return env;
}

Value* Environment::get_at(String name, u64 distance) {
    return ancestor(distance)->get(name);
}

void Environment::define_callable(Arena* arena, const String str, Callable in_callable) {
    callables.insert(arena, (void*) &str, sizeof(String), HASH_STR(str), &in_callable, sizeof(Callable));
}

Callable* Environment::get_callable(String name) {
    Callable* callable = (Callable*) callables.get(HASH_STR(name));
    if (callable != nullptr) {
        return callable;
    } else {
        if (enclosing != nullptr) {
            return enclosing->get_callable(name);
        }

        return nullptr;
    }
}

void Environment::define_class(Arena* arena, const String str, Class in_class) {
    classes.insert(arena, (void*) &str, sizeof(String), HASH_STR(str), &in_class, sizeof(Class));
}

Class* Environment::get_class(String name) {
    Class* clss = (Class*) classes.get(HASH_STR(name));
    if (clss != nullptr) {
        return clss;
    } else {
        if (enclosing != nullptr) {
            return enclosing->get_class(name);
        }

        return nullptr;
    }
}

void Class::print() const {
    fprintf(stdout, "%.*s instance\n", (u32) m_name.len, m_name.chars);
}