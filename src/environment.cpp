#include "environment.h"

void Environment::define(const Token* token, Value value) {
    define(token->m_lexeme, value);
}

void Environment::define(String str, Value value) {
    values[str] = value;
}

bool Environment::contains(const Token* token) const {
    if(values.contains(token->m_lexeme)) {
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

    if (values.contains(lexeme)) {
        values[lexeme] = value;
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

    if (values.contains(lexeme)) {
        in_value = values.at(lexeme);
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
    if (values.contains(name)) {
        return values.at(name);
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

void Environment::define_callable(const Token* token, Callable in_callable) {
    define_callable(token->m_lexeme, in_callable);
}

void Environment::define_callable(const String &str, Callable in_callable) {
    callables[str] = in_callable;
}

RuntimeError Environment::get_callable(const Token* token, Callable& in_callable) {
    String lexeme = token->m_lexeme;

    if (callables.contains(lexeme)) {
        in_callable = callables[lexeme];

        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            return enclosing->get_callable(token, in_callable);
        }

        return RuntimeError::undeclared_function(token);
    }
}

Callable* Environment::get_callable(String name) {
    if (callables.contains(name)) {
        return &callables[name];
    } else {
        if (enclosing != nullptr) {
            return enclosing->get_callable(name);
        }

        return nullptr;
    }
}

void Environment::define_class(const Token* token, Class in_class) {
    define_class(token->m_lexeme, in_class);
}

void Environment::define_class(const String &str, Class in_class) {
    classes[str] = in_class;
}

RuntimeError Environment::get_class(const Token* token, Class** in_class) {
    String lexeme = token->m_lexeme;

    if (classes.contains(lexeme)) {
        *in_class = &classes[lexeme];

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