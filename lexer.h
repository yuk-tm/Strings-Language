#ifndef LEXER_H
#define LEXER_H

// トークンの種類（正式仕様v0.11準拠）
typedef enum {
    TOKEN_EOF = 0,
    
    // 基本記号
    TOKEN_CMD_END,      // /
    TOKEN_BLOCK_END,    // //
    TOKEN_MULTI_CMD,    // ;
    TOKEN_IF,           // ?
    TOKEN_ELSE,         // !
    
    // 算術演算子（仕様準拠）
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_AT,           // @ (乗算)
    TOKEN_YEN,          // ¥ (除算)
    TOKEN_BACKSLASH,    // \ (除算)
    TOKEN_MOD,          // %
    
    // 比較演算子
    TOKEN_GT,           // >
    TOKEN_LT,           // <
    TOKEN_GTE,          // >=
    TOKEN_LTE,          // <=
    TOKEN_EQ,           // ==
    TOKEN_NEQ,          // !=
    
    // 論理演算子
    TOKEN_AMPERSAND,    // & (AND)
    TOKEN_PIPE,         // | (OR)
    TOKEN_TILDE,        // ~ (NOT)
    
    // その他の記号
    TOKEN_ASSIGN,       // =
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_COMMA,        // ,
    TOKEN_COLON,        // :
    TOKEN_DOT,          // . (リストアクセス用)
    TOKEN_CARET,        // ^ (範囲指定用)
    TOKEN_QUOTE,        // ' (数値プレフィックス)
    TOKEN_HASH,         // # (コメント)
    
    // データ
    TOKEN_IDENTIFIER,   // 変数名、文字列リテラル
    TOKEN_NUMBER,       // 数値（'5 形式）
    TOKEN_STRING,       // 文字列（クォートなし）
    
    // キーワード
    TOKEN_RE,           // re
    TOKEN_FUNC,         // func (将来用)
    TOKEN_WRITE,        // write
    TOKEN_RUN,          // run
    TOKEN_CALL,         // call
    TOKEN_SUNUM,        // sunum
    TOKEN_PY,           // py
    TOKEN_WINDOW,       // Window (GUI用)
    TOKEN_BUTTON,       // Button (GUI用)
    
    // エラー
    TOKEN_ERROR
} TokenType;

// トークン構造体
typedef struct {
    TokenType type;
    char* value;        // トークンの文字列値
    int line;           // 行番号
    int column;         // 列番号
} Token;

// トークンリスト
typedef struct {
    Token* tokens;
    int count;
    int capacity;
} TokenList;

// 字句解析器の状態
typedef struct {
    const char* input;  // 入力文字列
    int position;       // 現在の位置
    int line;           // 現在の行番号
    int column;         // 現在の列番号
    char current_char;  // 現在の文字
} Lexer;

// 関数宣言
TokenList tokenize(const char* input);
void free_tokens(TokenList* tokens);
const char* token_to_string(TokenType type);

// 字句解析器の内部関数
Lexer* lexer_create(const char* input);
void lexer_free(Lexer* lexer);
void lexer_advance(Lexer* lexer);
void lexer_skip_whitespace(Lexer* lexer);
void lexer_skip_comment(Lexer* lexer);
Token lexer_read_number(Lexer* lexer);
Token lexer_read_string(Lexer* lexer);
Token lexer_read_identifier(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);

#endif