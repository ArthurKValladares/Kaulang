#include "scanner.h"

#include <ctype.h>

void Scanner::init_keywords_map(Arena* arena) {
    keywords.allocate(arena);

    TokenType* ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::AND;
    keywords.insert(arena, CREATE_STRING("and"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::CLASS;
    keywords.insert(arena, CREATE_STRING("class"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::ELSE;
    keywords.insert(arena, CREATE_STRING("else"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::FN;
    keywords.insert(arena, CREATE_STRING("fn"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::STATIC;
    keywords.insert(arena, CREATE_STRING("static"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::FOR;
    keywords.insert(arena, CREATE_STRING("for"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::IF;
    keywords.insert(arena, CREATE_STRING("if"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::NIL;
    keywords.insert(arena, CREATE_STRING("nil"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::OR;
    keywords.insert(arena, CREATE_STRING("or"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::RETURN;
    keywords.insert(arena, CREATE_STRING("return"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::SUPER;
    keywords.insert(arena, CREATE_STRING("super"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::THIS;
    keywords.insert(arena, CREATE_STRING("this"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::TRUE;
    keywords.insert(arena, CREATE_STRING("true"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::FALSE;
    keywords.insert(arena, CREATE_STRING("false"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::VAR;
    keywords.insert(arena, CREATE_STRING("var"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::WHILE;
    keywords.insert(arena, CREATE_STRING("while"), ty);

    ty = (TokenType*) arena->push_struct_no_zero<TokenType>();
    *ty = TokenType::BREAK;
    keywords.insert(arena, CREATE_STRING("break"), ty);
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

void Scanner::add_token(Arena* arena, TokenType token_type, TokenData data) {
    m_tokens.push(Token{
        token_type,
        String{},
        m_current_line,
        data
    });
}

void Scanner::add_token(Arena* arena, TokenType token_type, String substr, TokenData data) {
    String lexeme = substr;
    if (substr.empty()) {
        lexeme = get_substring(m_start_char_offset, m_current_char_offset);
    }

    m_tokens.push(
        Token{
            token_type,
            lexeme,
            m_current_line,
            data
        }
    );
}

void Scanner::string(KauCompiler& compiler, Arena* arena) {
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
    add_token(arena, TokenType::STRING, substr);
}

void Scanner::number(KauCompiler& compiler, Arena* arena) {
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
        add_token(arena, TokenType::NUMBER_INT, TokenData::new_int(integer));
    } else if (ty == TokenData::Type::LONG) {
        const long integer = atol(m_source + m_start_char_offset);
        add_token(arena, TokenType::NUMBER_LONG, TokenData::new_long(integer));
    } else if (ty == TokenData::Type::FLOAT) {
        const float fractional = atof(m_source + m_start_char_offset);
        add_token(arena, TokenType::NUMBER_FLOAT, TokenData::new_float(fractional));
    } else {
        char* err;
        const double fractional = strtod(m_source + m_start_char_offset, &err);
        if (err == nullptr) {
            compiler.error(m_current_line, CREATE_STRING("could not convert string to double"));
        }
        add_token(arena, TokenType::NUMBER_DOUBLE, TokenData::new_double(fractional));
    }
}

void Scanner::identifier(Arena* arena) {
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }

    const String id = get_substring(m_start_char_offset, m_current_char_offset);

    TokenType* ty = (TokenType*)keywords.get(id);
    if (ty != nullptr) { 
        add_token(arena, *ty, id);
    } else {
        // TODO: This add_token stuff is a bit messy atm imo, specially the subcstring stuff.
        // MAybe just handle it all explicitly
        add_token(arena, TokenType::IDENTIFIER, String{});
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
            add_token(arena, TokenType::LEFT_PAREN);
            break;
        }
        case ')': {
            add_token(arena, TokenType::RIGHT_PAREN);
            break;
        }
        case '{': {
            add_token(arena, TokenType::LEFT_BRACE);
            break;
        }
        case '}': {
            add_token(arena, TokenType::RIGHT_BRACE);
            break;
        }
        case ',': {
            add_token(arena, TokenType::COMMA);
            break;
        }
        case '.': {
            add_token(arena, TokenType::DOT);
            break;
        }
        case '-': {
            add_token(arena, TokenType::MINUS);
            break;
        }
        case '+': {
            add_token(arena, TokenType::PLUS);
            break;
        }
        case ';': {
            add_token(arena, TokenType::SEMICOLON);
            break;
        }
        case ':': {
            add_token(arena, TokenType::COLON);
            break;
        }
        case '*': {
            add_token(arena, TokenType::STAR);
            break;
        }
        case '?': {
            add_token(arena, TokenType::QUESTION_MARK);
            break;
        }
        // One or two character tokens
        case '!': {
            add_token(arena, match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        }
        case '=': {
            add_token(arena, match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        }
        case '>': {
            add_token(arena, match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        }
        case '<': {
            add_token(arena, match('=') ? TokenType::LESSER_EQUAL : TokenType::LESSER);
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
                add_token(arena, TokenType::SLASH);
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
            string(compiler, arena);
            break;
        }
        case '\0': {
            break;
        }
        default: {
            if (isdigit(c)) {
                number(compiler, arena);
            } else if (isalpha(c) || c == '_') {
                identifier(arena);
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