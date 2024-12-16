#include "scanner.h"

#include <ctype.h>

#include <print>

// TODO: better way to do this lookup table
#include <unordered_map>
const std::unordered_map<std::string_view, TokenType> keywords = {
    {"and", TokenType::AND},
    {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},
    {"fn", TokenType::FN},
    {"for", TokenType::FOR},
    {"if", TokenType::IF},
    {"nil", TokenType::NIL},
    {"or", TokenType::OR},
    {"print", TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"var", TokenType::VAR},
    {"while", TokenType::WHILE},
};

bool Scanner::is_at_end() const {
    return m_current_char_offset >= m_source_len;
}

char Scanner::peek() const {
    if (is_at_end()) return '\0';
    return m_source[m_current_char_offset];
}

char Scanner::peek_next() const {
    if (m_current_char_offset + 1 >= m_source_len) {
        return '\0';
    }
    return m_source[m_current_char_offset + 1];
}

char Scanner::advance() {
    return m_source[m_current_char_offset++];
}

bool Scanner::match(char c) {
    if (is_at_end()) return false;
    if (m_source[m_current_char_offset] != c) return false;

    m_current_char_offset++;

    return true;
}

std::string_view Scanner::get_substring(int start_offset, int end_offset) const {
    const char* start_char = m_source + start_offset;
    const int substr_len = end_offset - start_offset;
    return std::string_view(start_char, substr_len);
}

void Scanner::add_token(TokenType token_type, TokenData data) {
    m_tokens.push_back(Token{
        token_type,
        std::string_view(),
        m_current_line,
        data
    });
}

void Scanner::add_token(TokenType token_type, std::string_view substr, TokenData data) {
    std::string_view lexeme = substr;
    if (substr.empty()) {
        lexeme = get_substring(m_start_char_offset, m_current_char_offset);
    }

    m_tokens.push_back(Token{
        token_type,
        lexeme,
        m_current_line,
        data
    });
}

void Scanner::string(KauCompiler& compiler) {
    while(!is_at_end() && peek() != '"') {
        if (peek() == '\n') {
            ++m_current_line;
        }
        advance();
    }
    if (is_at_end()) {
        compiler.error(m_current_line, "unterminated string");
        return;
    }

    advance();

    const std::string_view substr = get_substring(m_start_char_offset + 1, m_current_char_offset - 1);
    add_token(TokenType::STRING, substr);
}

void Scanner::number() {
    while(isdigit(peek())) {
        advance();
    }

    TokenData::Type ty = TokenData::Type::INT;

    if (peek() == '.') {
        advance(); // skip over '.'
        ty = TokenData::Type::FLOAT;

        while(isdigit(peek())) {
            advance();
        }
    }

    if (peek() == 'f') {
        advance();
        ty = TokenData::Type::FLOAT;
    } else if (peek() == 'd') {
        advance();
        ty = TokenData::Type::DOUBLE;
    }

    if (ty == TokenData::Type::INT) {
        const int integer = atoi(m_source + m_start_char_offset);
        add_token(TokenType::NUMBER_INT, TokenData::new_int(integer));
    } else if (ty == TokenData::Type::FLOAT) {
        const float fractional = atof(m_source + m_start_char_offset);
        add_token(TokenType::NUMBER_FLOAT, TokenData::new_float(fractional));
    } else {
        char* err;
        const double fractional = strtod(m_source + m_start_char_offset, &err);
        // TODO: Probably some error checking on err
        add_token(TokenType::NUMBER_DOUBLE, TokenData::new_double(fractional));
    }
}

void Scanner::identifier() {
    while (isalnum(peek())) {
        advance();
    }

    const std::string_view id = get_substring(m_start_char_offset, m_current_char_offset);

    if (keywords.contains(id)) {
        add_token(keywords.at(id));
    } else {
        add_token(TokenType::IDENTIFIER, std::string_view());
    }
}

void Scanner::block_comment(KauCompiler& compiler) {
    while(!is_at_end() && !(peek() == '*' && peek_next() == '/')) {
        if (peek() == '\n') {
            ++m_current_line;
        }
        advance();
    }

    if (is_at_end()) {
        compiler.error(m_current_line, "unterminated block-comment");
        return;
    }

    advance();
    advance();
}

void Scanner::scan_token(KauCompiler& compiler) {
    const char c = advance();
    switch(c) {
        // Single-character
        case '(': {
            add_token(TokenType::LEFT_PAREN);
            break;
        }
        case ')': {
            add_token(TokenType::RIGHT_PAREN);
            break;
        }
        case '{': {
            add_token(TokenType::LEFT_BRACE);
            break;
        }
        case '}': {
            add_token(TokenType::RIGHT_BRACE);
            break;
        }
        case ',': {
            add_token(TokenType::COMMA);
            break;
        }
        case '.': {
            add_token(TokenType::DOT);
            break;
        }
        case '-': {
            add_token(TokenType::MINUS);
            break;
        }
        case '+': {
            add_token(TokenType::PLUS);
            break;
        }
        case ';': {
            add_token(TokenType::SEMICOLON);
            break;
        }
        case ':': {
            add_token(TokenType::COLON);
            break;
        }
        case '*': {
            add_token(TokenType::STAR);
            break;
        }
        case '?': {
            add_token(TokenType::QUESTION_MARK);
            break;
        }
        // One or two character tokens
        case '!': {
            add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        }
        case '=': {
            add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        }
        case '>': {
            add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        }
        case '<': {
            add_token(match('=') ? TokenType::LESSER_EQUAL : TokenType::LESSER);
            break;
        }
        // Special-case
        case '/': {
            // Comment, so skip whole line
            if (match('/')) {
                while(!is_at_end() && peek() != '\n') {
                    advance();
                }
            } else if (match('*')) {
                block_comment(compiler);
            } else {
                add_token(TokenType::SLASH);
            }
            break;
        }
        // ignore white-space
        case ' ':
        case '\r':
        case '\t': {
            break;
        }
        // New line
        case '\n': {
            ++m_current_line;
            break;
        }
        // String literals
        case '"': {
            string(compiler);
            break;
        }
        case '\0': {
            break;
        }
        default: {
            if (isdigit(c)) {
                number();
            } else if (isalpha(c)) {
                identifier();
            } else {
                // TODO: variadic?
                compiler.error(m_current_line, "unexpected character {}");
            }
            break;
        }
    }
}

void Scanner::scan_tokens(KauCompiler& compiler) {
    while (!is_at_end()) {
        m_start_char_offset = m_current_char_offset;
        scan_token(compiler);
    }

    m_tokens.push_back(Token{
        TokenType::_EOF,
        "",
        m_current_line,
    });
}

#ifdef DEBUG
void Scanner::print_tokens() {
    for (const Token& token: m_tokens) {
        token.print();
        std::println("");
    }
}
#endif