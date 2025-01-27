#include "environment.h"

void Environment::define(const Token* token, Value value) {
    values[token->m_lexeme] = value;
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

void Class::print() const {
    std::println("{} instance", m_name.to_string_view());
}