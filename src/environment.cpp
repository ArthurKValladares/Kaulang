#include "environment.h"

void Environment::init(Arena* arena) {
    values.allocate(arena);
    callables.allocate(arena);
    classes.allocate(arena);
}

void Environment::define(Arena* arena, const Token* token, Value in_value) {
    define(arena, token->m_lexeme, in_value);
}

void Environment::define(Arena* arena, String str, Value in_value) {
    // TODO: This copy here could be inneficient. Same for the others below
    Value* val = (Value*) arena->push_struct_no_zero<Value>();
    *val = in_value;

    values.insert(arena, HASH_STR(str), val);
}

bool Environment::contains(const Token* token) const {
    const Value* val = (const Value*) values.get(HASH_STR(token->m_lexeme));
    if(val != nullptr) {
        return true;
    } else {
        if (enclosing != nullptr) {
            return enclosing->contains(token);
        }

        return false;
    }
}

RuntimeError Environment::set(const Token* token, Value value) {
    String lexeme = token->m_lexeme;

    Value* val = (Value*) values.get(HASH_STR(lexeme));
    if (val != nullptr) {
        *val = value;
        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            enclosing->set(token, value);
        }

        return RuntimeError::undeclared_variable(token);
    }
}

RuntimeError Environment::get(const Token* token, Value& in_value) {
    String lexeme = token->m_lexeme;

    Value* val = (Value*) values.get(HASH_STR(lexeme));
    if (val != nullptr) {
        in_value = *val;
        if (in_value.ty == Value::Type::NIL) {
            return RuntimeError::undefined_variable(token);
        }
        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            return enclosing->get(token, in_value);
        }

        return RuntimeError::undeclared_variable(token);
    }
}

// TODO: Maybe version without the check, get at root.
Value Environment::get_unchecked(String name) {
    Value* val = (Value*) values.get(HASH_STR(name));
    if (val != nullptr) {
        return *val;
    } else {
        return enclosing->get_unchecked(name);
    }
}

Environment* Environment::ancestor(u64 distance) {
    Environment* env = this;
    for (u64 i = 0; i < distance; ++i) {
        env = env->enclosing;
    }
    return env;
}

RuntimeError Environment::get_at(const Token* token, u64 distance, Value& in_value) {
    return ancestor(distance)->get(token, in_value);
}

void Environment::define_callable(Arena* arena, const Token* token, Callable in_callable) {
    define_callable(arena, token->m_lexeme, in_callable);
}

void Environment::define_callable(Arena* arena, const String &str, Callable in_callable) {
    Callable* callable = (Callable*) arena->push_struct_no_zero<Callable>();
    *callable = in_callable;

    callables.insert(arena, HASH_STR(str), callable);
}

RuntimeError Environment::get_callable(const Token* token, Callable& in_callable) {
    String lexeme = token->m_lexeme;

    Callable* callable = (Callable*) callables.get(HASH_STR(lexeme));
    if (callable != nullptr) {
        in_callable = *callable;

        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            return enclosing->get_callable(token, in_callable);
        }

        return RuntimeError::undeclared_function(token);
    }
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

void Environment::define_class(Arena* arena, const Token* token, Class in_class) {
    define_class(arena, token->m_lexeme, in_class);
}

void Environment::define_class(Arena* arena, const String &str, Class in_class) {
    Class* clss = (Class*) arena->push_struct_no_zero<Class>();
    *clss = in_class;

    classes.insert(arena, HASH_STR(str), clss);
}

RuntimeError Environment::get_class(const Token* token, Class** in_class) {
    String lexeme = token->m_lexeme;

    Class* clss = (Class*) classes.get(HASH_STR(lexeme));
    if (clss != nullptr) {
        *in_class = clss;

        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            return enclosing->get_class(token, in_class);
        }

        return RuntimeError::undeclared_function(token);
    }
}

void Class::print() const {
    fprintf(stdout, "%.*s instance\n", (u32) m_name.len, m_name.chars);
}