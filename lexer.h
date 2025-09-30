#ifndef LEXER_H
#define LEXER_H

// トークン（コードの部品）の種類
typedef enum {
    // 記号・演算子
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_MOD,
    TOKEN_GT, TOKEN_LT, TOKEN_GTE, TOKEN_LTE, TOKEN_EQ, TOKEN_NEQ,
    TOKEN_AMPERSAND, TOKEN_PIPE, TOKEN_TILDE,
    TOKEN_IF, TOKEN_ELSE,
    TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_MULTI_CMD, // ;
    TOKEN_CMD_END,   // /
    TOKEN_ASSIGN,    // =
    TOKEN_AT,        // @
    TOKEN_YEN,       // YEN (未使用可)
    TOKEN_BACKSLASH, // \

    // データ型
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // キーワード
    TOKEN_WRITE, TOKEN_NUM, TOKEN_RE, TOKEN_SUNUM,
    TOKEN_RUN, TOKEN_CALL, TOKEN_PY, TOKEN_FUNC, TOKEN_BLOCK_END,

    // その他
    TOKEN_COMMENT, TOKEN_ERROR, TOKEN_EOF
} TokenType;

// トークン構造体
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
    const char* source;
    int position;
    int line;
    int column;
    char current_char;
} Lexer;

// 関数宣言
Lexer* lexer_create(const char* source);
void lexer_free(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);
TokenList tokenize(const char* source);
void free_tokens(TokenList* tokens);
const char* token_to_string(TokenType type);

#endif