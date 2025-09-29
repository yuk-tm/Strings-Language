#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

const char* token_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_CMD_END: return "/";
        case TOKEN_BLOCK_END: return "//";
        case TOKEN_MULTI_CMD: return ";";
        case TOKEN_IF: return "?";
        case TOKEN_ELSE: return "!";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_AT: return "@";
        case TOKEN_YEN: return "¥";
        case TOKEN_BACKSLASH: return "\\";
        case TOKEN_MOD: return "%";
        case TOKEN_GT: return ">";
        case TOKEN_LT: return "<";
        case TOKEN_GTE: return ">=";
        case TOKEN_LTE: return "<=";
        case TOKEN_EQ: return "==";
        case TOKEN_NEQ: return "!=";
        case TOKEN_AMPERSAND: return "&";
        case TOKEN_PIPE: return "|";
        case TOKEN_TILDE: return "~";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_COMMA: return ",";
        case TOKEN_COLON: return ":";
        case TOKEN_DOT: return ".";
        case TOKEN_CARET: return "^";
        case TOKEN_QUOTE: return "'";
        case TOKEN_HASH: return "#";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_RE: return "re";
        case TOKEN_FUNC: return "func";
        case TOKEN_WRITE: return "write";
        case TOKEN_RUN: return "run";
        case TOKEN_CALL: return "call";
        case TOKEN_SUNUM: return "sunum";
        case TOKEN_PY: return "py";
        case TOKEN_WINDOW: return "Window";
        case TOKEN_BUTTON: return "Button";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

Lexer* lexer_create(const char* input) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = input[0];
    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (lexer) {
        free(lexer);
    }
}

void lexer_advance(Lexer* lexer) {
    if (lexer->current_char == '\0') return;

    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    lexer->position++;
    lexer->current_char = lexer->input[lexer->position];
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) {
        lexer_advance(lexer);
    }
}

Token lexer_create_token(TokenType type, const char* start, int length, int line, int column) {
    Token token;
    token.type = type;
    token.value = malloc(length + 1);
    strncpy(token.value, start, length);
    token.value[length] = '\0';
    token.line = line;
    token.column = column;
    return token;
}

Token lexer_read_number(Lexer* lexer) {
    int start_pos = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (isdigit(lexer->current_char) || lexer->current_char == '.') {
        lexer_advance(lexer);
    }
    
    int length = lexer->position - start_pos;
    return lexer_create_token(TOKEN_NUMBER, lexer->input + start_pos, length, start_line, start_column);
}

Token lexer_read_string(Lexer* lexer) {
    int start_pos = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (lexer->current_char != '\0' && !isspace(lexer->current_char) &&
           lexer->current_char != '/' && lexer->current_char != ';') {
        lexer_advance(lexer);
    }
    
    int length = lexer->position - start_pos;
    return lexer_create_token(TOKEN_STRING, lexer->input + start_pos, length, start_line, start_column);
}

Token lexer_read_identifier_or_keyword(Lexer* lexer) {
    int start_pos = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (lexer->current_char != '\0' && (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        lexer_advance(lexer);
    }
    
    int length = lexer->position - start_pos;
    char* value = malloc(length + 1);
    strncpy(value, lexer->input + start_pos, length);
    value[length] = '\0';
    
    TokenType type;
    
    if (strcmp(value, "re") == 0) {
        type = TOKEN_RE;
    } else if (strcmp(value, "func") == 0) {
        type = TOKEN_FUNC;
    } else if (strcmp(value, "write") == 0) {
        type = TOKEN_WRITE;
    } else if (strcmp(value, "run") == 0) {
        type = TOKEN_RUN;
    } else if (strcmp(value, "call") == 0) {
        type = TOKEN_CALL;
    } else if (strcmp(value, "sunum") == 0) {
        type = TOKEN_SUNUM;
    } else if (strcmp(value, "py") == 0) {
        type = TOKEN_PY;
    } else if (strcmp(value, "Window") == 0) {
        type = TOKEN_WINDOW;
    } else if (strcmp(value, "Button") == 0) {
        type = TOKEN_BUTTON;
    } else {
        type = TOKEN_IDENTIFIER;
    }

    Token token;
    token.type = type;
    token.value = value;
    token.line = start_line;
    token.column = start_column;
    return token;
}

Token lexer_next_token(Lexer* lexer) {
    lexer_skip_whitespace(lexer);

    int start_line = lexer->line;
    int start_column = lexer->column;

    if (lexer->current_char == '\0') {
        Token eof_token;
        eof_token.type = TOKEN_EOF;
        eof_token.value = strdup("");
        eof_token.line = start_line;
        eof_token.column = start_column;
        return eof_token;
    }
    
    if (isdigit(lexer->current_char)) {
        return lexer_read_number(lexer);
    }
    
    if (isalpha(lexer->current_char) || lexer->current_char == '_') {
        return lexer_read_identifier_or_keyword(lexer);
    }
    
    if (lexer->current_char == '\'') {
        lexer_advance(lexer);
        return lexer_read_number(lexer);
    }

    switch (lexer->current_char) {
        case '#':
            while (lexer->current_char != '\0' && lexer->current_char != '\n') {
                lexer_advance(lexer);
            }
            return lexer_next_token(lexer);
        case '/':
            lexer_advance(lexer);
            if (lexer->current_char == '/') {
                lexer_advance(lexer);
                return lexer_create_token(TOKEN_BLOCK_END, "//", 2, start_line, start_column);
            }
            return lexer_create_token(TOKEN_CMD_END, "/", 1, start_line, start_column);
        case ';':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_MULTI_CMD, ";", 1, start_line, start_column);
        case '?':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_IF, "?", 1, start_line, start_column);
        case '!':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                lexer_advance(lexer);
                return lexer_create_token(TOKEN_NEQ, "!=", 2, start_line, start_column);
            }
            return lexer_create_token(TOKEN_ELSE, "!", 1, start_line, start_column);
        case '+':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_PLUS, "+", 1, start_line, start_column);
        case '-':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_MINUS, "-", 1, start_line, start_column);
        case '@':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_AT, "@", 1, start_line, start_column);
        case '¥':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_YEN, "¥", 1, start_line, start_column);
        case '\\':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_BACKSLASH, "\\", 1, start_line, start_column);
        case '%':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_MOD, "%", 1, start_line, start_column);
        case '>':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                lexer_advance(lexer);
                return lexer_create_token(TOKEN_GTE, ">=", 2, start_line, start_column);
            }
            return lexer_create_token(TOKEN_GT, ">", 1, start_line, start_column);
        case '<':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                lexer_advance(lexer);
                return lexer_create_token(TOKEN_LTE, "<=", 2, start_line, start_column);
            }
            return lexer_create_token(TOKEN_LT, "<", 1, start_line, start_column);
        case '=':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                lexer_advance(lexer);
                return lexer_create_token(TOKEN_EQ, "==", 2, start_line, start_column);
            }
            return lexer_create_token(TOKEN_ASSIGN, "=", 1, start_line, start_column);
        case '&':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_AMPERSAND, "&", 1, start_line, start_column);
        case '|':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_PIPE, "|", 1, start_line, start_column);
        case '~':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_TILDE, "~", 1, start_line, start_column);
        case '(':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_LPAREN, "(", 1, start_line, start_column);
        case ')':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_RPAREN, ")", 1, start_line, start_column);
        case ',':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_COMMA, ",", 1, start_line, start_column);
        case ':':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_COLON, ":", 1, start_line, start_column);
        case '.':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_DOT, ".", 1, start_line, start_column);
        case '^':
            lexer_advance(lexer);
            return lexer_create_token(TOKEN_CARET, "^", 1, start_line, start_column);
        default:
            if (isprint(lexer->current_char)) {
                return lexer_read_string(lexer);
            } else {
                Token token;
                token.type = TOKEN_ERROR;
                token.value = malloc(2);
                token.value[0] = lexer->current_char;
                token.value[1] = '\0';
                lexer_advance(lexer);
                return token;
            }
    }
}

void free_tokens(TokenList* tokens) {
    if (tokens->tokens) {
        for (int i = 0; i < tokens->count; i++) {
            if (tokens->tokens[i].value) {
                free(tokens->tokens[i].value);
            }
        }
        free(tokens->tokens);
        tokens->tokens = NULL;
        tokens->count = 0;
        tokens->capacity = 0;
    }
}

TokenList tokenize(const char* input) {
    TokenList tokens;
    tokens.tokens = malloc(sizeof(Token) * 100);
    tokens.count = 0;
    tokens.capacity = 100;
    
    Lexer* lexer = lexer_create(input);
    
    while (1) {
        Token token = lexer_next_token(lexer);
        
        if (tokens.count >= tokens.capacity) {
            tokens.capacity *= 2;
            tokens.tokens = realloc(tokens.tokens, sizeof(Token) * tokens.capacity);
        }
        
        tokens.tokens[tokens.count++] = token;
        
        if (token.type == TOKEN_EOF) {
            break;
        }
    }
    
    lexer_free(lexer);
    return tokens;
}