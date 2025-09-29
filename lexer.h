#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EOF = 0,
    
    TOKEN_CMD_END,
    TOKEN_BLOCK_END,
    TOKEN_MULTI_CMD,
    TOKEN_IF,
    TOKEN_ELSE,
    
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_AT,
    TOKEN_YEN,
    TOKEN_BACKSLASH,
    TOKEN_MOD,
    
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_GTE,
    TOKEN_LTE,
    TOKEN_EQ,
    TOKEN_NEQ,
    
    TOKEN_AMPERSAND,
    TOKEN_PIPE,
    TOKEN_TILDE,
    
    TOKEN_ASSIGN,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_CARET,
    TOKEN_QUOTE,
    TOKEN_HASH,
    
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    
    TOKEN_RE,
    TOKEN_FUNC,
    TOKEN_WRITE,
    TOKEN_RUN,
    TOKEN_CALL,
    TOKEN_SUNUM,
    TOKEN_PY,
    TOKEN_WINDOW,
    TOKEN_BUTTON,
    
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

typedef struct {
    Token* tokens;
    int count;
    int capacity;
} TokenList;

typedef struct {
    const char* input;
    int position;
    int line;
    int column;
    char current_char;
} Lexer;

const char* token_to_string(TokenType type);

Lexer* lexer_create(const char* input);
void lexer_free(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);
TokenList tokenize(const char* input);
void free_tokens(TokenList* tokens);

#endif