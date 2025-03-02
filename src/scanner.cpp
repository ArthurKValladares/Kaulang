#include "scanner.h"

#include <ctype.h>

#define ADD_TO_KEYWORDS(TYPE, STRING) do {\
    String str = CREATE_STRING(STRING);\
    const TokenType ty = TokenType::TYPE;\
    keywords.insert(arena, str, HASH_STR(str), ty);\
} while(0)

namespace {
    Map keywords;
};

void init_keywords_map(Arena* arena) {
    keywords.allocate(arena);

    ADD_TO_KEYWORDS(AND, "and");
    ADD_TO_KEYWORDS(CLASS, "class");
    ADD_TO_KEYWORDS(ELSE, "else");
    ADD_TO_KEYWORDS(FN, "fn");
    ADD_TO_KEYWORDS(STATIC, "static");
    ADD_TO_KEYWORDS(FOR, "for");
    ADD_TO_KEYWORDS(IF, "if");
    ADD_TO_KEYWORDS(NIL, "nil");
    ADD_TO_KEYWORDS(OR, "or");
    ADD_TO_KEYWORDS(RETURN, "return");
    ADD_TO_KEYWORDS(SUPER, "super");
    ADD_TO_KEYWORDS(THIS, "this");
    ADD_TO_KEYWORDS(TRUE, "true");
    ADD_TO_KEYWORDS(FALSE, "false");
    ADD_TO_KEYWORDS(VAR, "var");
    ADD_TO_KEYWORDS(WHILE, "while");
    ADD_TO_KEYWORDS(BREAK, "break");
}

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

String Scanner::get_substring(int start_offset, int end_offset) const {
    const char* start_char = m_source + start_offset;
    const size_t substr_len = end_offset - start_offset;
    return String{start_char, substr_len};
}

void Scanner::add_token(TokenType token_type, String substr, TokenData data) {
    m_tokens.push(
        Token{
            token_type,
            substr,
            m_current_line,
            data
        }
    );
}

void Scanner::string(KauCompiler& compiler) {
    while(!is_at_end() && peek() != '"') {
        if (peek() == '\n') {
            ++m_current_line;
        }
        advance();
    }
    if (is_at_end()) {
        compiler.error(m_current_line, CREATE_STRING("unterminated string"));
        return;
    }

    advance();

    const String substr = get_substring(m_start_char_offset + 1, m_current_char_offset - 1);
    add_token(TokenType::STRING, substr);
}

void Scanner::number(KauCompiler& compiler) {
    while(isdigit(peek())) {
        advance();
    }

    TokenData::Type ty = TokenData::Type::INT;
    if (peek() == 'l') {
        ty = TokenData::Type::LONG;
    }

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
        add_token(TokenType::NUMBER_INT, String{}, TokenData::new_int(integer));
    } else if (ty == TokenData::Type::LONG) {
        const long integer = atol(m_source + m_start_char_offset);
        add_token(TokenType::NUMBER_LONG, String{}, TokenData::new_long(integer));
    } else if (ty == TokenData::Type::FLOAT) {
        const float fractional = atof(m_source + m_start_char_offset);
        add_token(TokenType::NUMBER_FLOAT, String{}, TokenData::new_float(fractional));
    } else {
        char* err;
        const double fractional = strtod(m_source + m_start_char_offset, &err);
        if (err == nullptr) {
            compiler.error(m_current_line, CREATE_STRING("could not convert string to double"));
        }
        add_token(TokenType::NUMBER_DOUBLE, String{}, TokenData::new_double(fractional));
    }
}

void Scanner::identifier() {
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }

    const String id = get_substring(m_start_char_offset, m_current_char_offset);

    TokenType* ty = (TokenType*)keywords.get(HASH_STR(id));
    if (ty != nullptr) { 
        add_token(*ty, id);
    } else {
        String substr = get_substring(m_start_char_offset, m_current_char_offset);
        add_token(TokenType::IDENTIFIER, substr);
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
        compiler.error(m_current_line, CREATE_STRING("unterminated block-comment"));
        return;
    }

    advance();
    advance();
}

void Scanner::scan_token(KauCompiler& compiler, Arena* arena) {
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
                number(compiler);
            } else if (isalpha(c) || c == '_') {
                identifier();
            } else {
                compiler.error(m_current_line,  concatenated_string(arena, CREATE_STRING("unexpected character "), String{&c, 1}));
            }
            break;
        }
    }
}

void Scanner::scan_tokens(KauCompiler& compiler, Arena* arena) {
    while (!is_at_end()) {
        m_start_char_offset = m_current_char_offset;
        scan_token(compiler, arena);
    }

    m_tokens.push(
        Token{
            TokenType::_EOF,
            CREATE_STRING(""),
            m_current_line,
        }
    );
}

#ifdef DEBUG
void Scanner::print_tokens() {
    for (size_t i = 0; i < m_tokens.size(); ++i) {
        Token& token = m_tokens[i];
        token.print();
        fprintf(stdout, "\n");
    }
}
#endif