#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// strndupの代用
static char* my_strndup(const char* s, size_t n) {
    size_t len = 0;
    while (len < n && s[len] != '\0') len++;
    char* new_s = (char*)malloc(len + 1);
    if (!new_s) return NULL;
    memcpy(new_s, s, len);
    new_s[len] = '\0';
    return new_s;
}

static Token create_token(TokenType type, char* value, int line, int col) {
    return (Token){type, value, line, col};
}

Lexer* lexer_create(const char* source) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = source[0];
    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (lexer) free(lexer);
}

static void lexer_advance(Lexer* lexer) {
    if (lexer->current_char == '\n') {
        lexer->line++; lexer->column = 1;
    } else {
        lexer->column++;
    }
    lexer->position++;
    lexer->current_char = lexer->source[lexer->position];
}

static void lexer_skip_whitespace(Lexer* lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) lexer_advance(lexer);
}

static Token lexer_read_identifier(Lexer* lexer) {
    int start = lexer->position;
    int line = lexer->line;
    int col = lexer->column;
    while (lexer->current_char != '\0' && (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        lexer_advance(lexer);
    }
    int len = lexer->position - start;
    char* value = my_strndup(lexer->source + start, len);

    // キーワード判定
    if (strcmp(value, "write") == 0) return create_token(TOKEN_WRITE, value, line, col);
    if (strcmp(value, "num") == 0) return create_token(TOKEN_NUM, value, line, col);
    if (strcmp(value, "re") == 0) return create_token(TOKEN_RE, value, line, col);
    if (strcmp(value, "sunum") == 0) return create_token(TOKEN_SUNUM, value, line, col);
    if (strcmp(value, "run") == 0) return create_token(TOKEN_RUN, value, line, col);
    if (strcmp(value, "call") == 0) return create_token(TOKEN_CALL, value, line, col);
    if (strcmp(value, "py") == 0) return create_token(TOKEN_PY, value, line, col);
    if (strcmp(value, "func") == 0) return create_token(TOKEN_FUNC, value, line, col);
    if (strcmp(value, "end") == 0) return create_token(TOKEN_BLOCK_END, value, line, col);

    return create_token(TOKEN_IDENTIFIER, value, line, col);
}

static Token lexer_read_quoted(Lexer* lexer, char quote_type) {
    int line = lexer->line;
    int col = lexer->column;
    lexer_advance(lexer); // skip start quote
    int start = lexer->position;
    while (lexer->current_char != '\0' && lexer->current_char != quote_type) {
        lexer_advance(lexer);
    }
    int len = lexer->position - start;
    char* value = my_strndup(lexer->source + start, len);

    if (lexer->current_char == quote_type) {
        lexer_advance(lexer);
        if (quote_type == '\'') return create_token(TOKEN_NUMBER, value, line, col);
        else if (quote_type == '"') return create_token(TOKEN_STRING, value, line, col);
    }
    free(value);
    return create_token(TOKEN_ERROR, strdup("Unclosed quote"), line, col);
}

Token lexer_next_token(Lexer* lexer) {
    lexer_skip_whitespace(lexer);
    int line = lexer->line;
    int col = lexer->column;
    if (lexer->current_char == '\0') return create_token(TOKEN_EOF, NULL, line, col);

    if (isalpha(lexer->current_char) || lexer->current_char == '_') return lexer_read_identifier(lexer);
    if (lexer->current_char == '\'' || lexer->current_char == '"') return lexer_read_quoted(lexer, lexer->current_char);

    // コメント
    if (lexer->current_char == '#') {
        lexer_advance(lexer);
        int start = lexer->position;
        while (lexer->current_char != '\0' && lexer->current_char != '\n') lexer_advance(lexer);
        int len = lexer->position - start;
        char* value = my_strndup(lexer->source + start, len);
        return create_token(TOKEN_COMMENT, value, line, col);
    }

    char current_char = lexer->current_char;
    lexer_advance(lexer);
    switch (current_char) {
        case '+': if (lexer->current_char == '*') { lexer_advance(lexer); return create_token(TOKEN_MULTIPLY, strdup("+*"), line, col);} return create_token(TOKEN_PLUS, strdup("+"), line, col);
        case '-': if (lexer->current_char == '*') { lexer_advance(lexer); return create_token(TOKEN_DIVIDE, strdup("-*"), line, col);} return create_token(TOKEN_MINUS, strdup("-"), line, col);
        case '=': if (lexer->current_char == '=') { lexer_advance(lexer); return create_token(TOKEN_EQ, strdup("=="), line, col);} return create_token(TOKEN_ASSIGN, strdup("="), line, col);
        case '!': if (lexer->current_char == '=') { lexer_advance(lexer); return create_token(TOKEN_NEQ, strdup("!="), line, col);} return create_token(TOKEN_ELSE, strdup("!"), line, col);
        case '>': if (lexer->current_char == '=') { lexer_advance(lexer); return create_token(TOKEN_GTE, strdup(">="), line, col);} return create_token(TOKEN_GT, strdup(">"), line, col);
        case '<': if (lexer->current_char == '=') { lexer_advance(lexer); return create_token(TOKEN_LTE, strdup("<="), line, col);} return create_token(TOKEN_LT, strdup("<"), line, col);
        case '%': return create_token(TOKEN_MOD, strdup("%"), line, col);
        case '&': return create_token(TOKEN_AMPERSAND, strdup("&"), line, col);
        case '|': return create_token(TOKEN_PIPE, strdup("|"), line, col);
        case '~': return create_token(TOKEN_TILDE, strdup("~"), line, col);
        case '?': return create_token(TOKEN_IF, strdup("?"), line, col);
        case '(': return create_token(TOKEN_LPAREN, strdup("("), line, col);
        case ')': return create_token(TOKEN_RPAREN, strdup(")"), line, col);
        case ';': return create_token(TOKEN_MULTI_CMD, strdup(";"), line, col);
        case '/': return create_token(TOKEN_CMD_END, strdup("/"), line, col);
        case '@': return create_token(TOKEN_AT, strdup("@"), line, col);
        case '\\': return create_token(TOKEN_BACKSLASH, strdup("\\"), line, col);
    }
    char unknown[2] = {current_char, '\0'};
    return create_token(TOKEN_ERROR, strdup(unknown), line, col);
}

TokenList tokenize(const char* source) {
    Lexer* lexer = lexer_create(source);
    TokenList tokens;
    tokens.count = 0;
    tokens.capacity = 10;
    tokens.tokens = malloc(sizeof(Token) * tokens.capacity);
    while (1) {
        Token token = lexer_next_token(lexer);
        if (tokens.count >= tokens.capacity) {
            tokens.capacity *= 2;
            tokens.tokens = realloc(tokens.tokens, sizeof(Token) * tokens.capacity);
            if (!tokens.tokens) { fprintf(stderr, "Fatal error: Failed to reallocate tokens.\n"); exit(1);}
        }
        tokens.tokens[tokens.count++] = token;
        if (token.type == TOKEN_EOF || token.type == TOKEN_ERROR) break;
    }
    lexer_free(lexer);
    return tokens;
}

void free_tokens(TokenList* tokens) {
    if (!tokens) return;
    for (int i = 0; i < tokens->count; i++) {
        if (tokens->tokens[i].value) free(tokens->tokens[i].value);
    }
    free(tokens->tokens);
    tokens->tokens = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}

const char* token_to_string(TokenType type) {
    switch (type) {
        case TOKEN_PLUS: return "'+'";
        case TOKEN_MINUS: return "'-'";
        case TOKEN_MOD: return "'%'";
        case TOKEN_GT: return "'>'";
        case TOKEN_LT: return "'<'";
        case TOKEN_AMPERSAND: return "'&'";
        case TOKEN_PIPE: return "'|'";
        case TOKEN_TILDE: return "'~'";
        case TOKEN_IF: return "'?'";
        case TOKEN_ELSE: return "'!'";
        case TOKEN_LPAREN: return "'('";
        case TOKEN_RPAREN: return "')'";
        case TOKEN_MULTI_CMD: return "';'";
        case TOKEN_CMD_END: return "'/'";
        case TOKEN_ASSIGN: return "'='";
        case TOKEN_EQ: return "'=='";
        case TOKEN_NEQ: return "'!='";
        case TOKEN_GTE: return "'>='";
        case TOKEN_LTE: return "'<='";
        case TOKEN_MULTIPLY: return "'+*'";
        case TOKEN_DIVIDE: return "'-*'";
        case TOKEN_AT: return "'@'";
        case TOKEN_YEN: return "'YEN'";
        case TOKEN_BACKSLASH: return "'\\'";
        case TOKEN_WRITE: return "WRITE";
        case TOKEN_NUM: return "NUM";
        case TOKEN_RE: return "RE";
        case TOKEN_SUNUM: return "SUNUM";
        case TOKEN_RUN: return "RUN";
        case TOKEN_CALL: return "CALL";
        case TOKEN_PY: return "PY";
        case TOKEN_FUNC: return "FUNC";
        case TOKEN_BLOCK_END: return "BLOCK_END ('end')";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_EOF: return "EOF";
        default: return "UNKNOWN_TOKEN";
    }
}