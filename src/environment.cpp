#include "environment.h"

void Environment::define(const Token* token, Value value) {
    // TODO: allocation
    values[std::string(token->m_lexeme)] = value;
}

bool Environment::contains(const Token* token) const {
    std::string str_name = std::string(token->m_lexeme);
    if(values.contains(str_name)) {
        return true;
    } else {
        if (enclosing != nullptr) {
            return enclosing->contains(token);
        }

        return false;
    }
}

RuntimeError Environment::set(const Token* token, Value value) {
    std::string str_name = std::string(token->m_lexeme);

    if (values.contains(str_name)) {
        values[str_name] = value;
        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            enclosing->set(token, value);
        }

        return RuntimeError::undeclared_variable(token);
    }
}

RuntimeError Environment::get(const Token* token, Value& in_value) {
    std::string str_name = std::string(token->m_lexeme);

    if (values.contains(str_name)) {
        in_value = values.at(str_name);
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

void Environment::define_callable(const Token* token, Callable in_callable) {
    // TODO: Allocation
    define_callable(std::string(token->m_lexeme), in_callable);
}

void Environment::define_callable(const std::string &str, Callable in_callable) {
    callables[str] = in_callable;
}

RuntimeError Environment::get_callable(const Token* token, Callable& in_callable) {
    std::string str_name = std::string(token->m_lexeme);

    if (callables.contains(str_name)) {
        in_callable = callables[str_name];

        return RuntimeError::ok();
    } else {
        if (enclosing != nullptr) {
            return enclosing->get_callable(token, in_callable);
        }

        return RuntimeError::undeclared_function(token);
    }
}