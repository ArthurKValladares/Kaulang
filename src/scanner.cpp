#include "scanner.h"

bool Scanner::is_at_end() const {
    return m_current_char_offset >= m_source_len;
}

char Scanner::peek() const {
    if (is_at_end()) return '\0';
    return m_source[m_current_char_offset];
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

void Scanner::add_token(TokenType token_type) {
        const char* start_char = m_source + m_start_char_offset;
        const int substr_len = m_current_char_offset - m_start_char_offset;

        const std::string_view substr = std::string_view(start_char, substr_len);

        m_tokens.push_back(Token{
            token_type,
            substr,
            m_current_line, 
        });
    }

void Scanner::scan_token() {
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
        case '*': {
            add_token(TokenType::STAR);
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
            add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        }
        // Special-case
        case '/': {
            // Comment, so skip whole line
            if (match('/')) {
                while(!is_at_end() && peek() != '\n') {
                    advance();
                }
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
        // TODO: is this case necessary?
        case '\0': {
            break;
        }
        default: {
            // TODO: Hook-up the error stuff
            fprintf(stderr, "Error, unexpected character: %c\n", c);
            break;
        }
    }
}

void Scanner::scan_tokens() {
    while (!is_at_end()) {
        m_start_char_offset = m_current_char_offset;
        scan_token();
    }

    m_tokens.push_back(Token{
        TokenType::_EOF,
        "",
        m_current_line,
    });
}