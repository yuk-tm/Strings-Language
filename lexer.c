#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// トークン名を文字列で返す（デバッグ用）
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
        case TOKEN_IDENTIFIER: return "ID";
        case TOKEN_NUMBER: return "NUM";
        case TOKEN_STRING: return "STR";
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

// 字句解析器を作成
Lexer* lexer_create(const char* input) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = input[0];
    return lexer;
}

// 字句解析器を解放
void lexer_free(Lexer* lexer) {
    if (lexer) {
        free(lexer);
    }
}

// 次の文字に進む
void lexer_advance(Lexer* lexer) {
    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    
    lexer->position++;
    if (lexer->position >= strlen(lexer->input)) {
        lexer->current_char = '\0';  // EOF
    } else {
        lexer->current_char = lexer->input[lexer->position];
    }
}

// 空白文字をスキップ
void lexer_skip_whitespace(Lexer* lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) {
        lexer_advance(lexer);
    }
}

// コメント（# から行末まで）をスキップ
void lexer_skip_comment(Lexer* lexer) {
    while (lexer->current_char != '\0' && lexer->current_char != '\n') {
        lexer_advance(lexer);
    }
}

// 数値を読み込み（'5 形式）
Token lexer_read_number(Lexer* lexer) {
    Token token;
    token.type = TOKEN_NUMBER;
    token.line = lexer->line;
    token.column = lexer->column;
    
    // ' をスキップ（既にチェック済み）
    lexer_advance(lexer);
    
    char buffer[100];
    int i = 0;
    
    // 数字を読み込み
    while (lexer->current_char != '\0' && (isdigit(lexer->current_char) || lexer->current_char == '.')) {
        buffer[i++] = lexer->current_char;
        lexer_advance(lexer);
    }
    buffer[i] = '\0';
    
    token.value = malloc(strlen(buffer) + 1);
    strcpy(token.value, buffer);
    
    return token;
}

// 変数識別子を読み込み（"name 形式）
Token lexer_read_variable(Lexer* lexer) {
    Token token;
    token.type = TOKEN_IDENTIFIER;
    token.line = lexer->line;
    token.column = lexer->column;
    
    // " をスキップ
    lexer_advance(lexer);
    
    char buffer[100];
    int i = 0;
    
    // 英数字、アンダースコアを読み込み
    while (lexer->current_char != '\0' && 
           (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        buffer[i++] = lexer->current_char;
        lexer_advance(lexer);
    }
    buffer[i] = '\0';
    
    token.value = malloc(strlen(buffer) + 1);
    strcpy(token.value, buffer);
    
    return token;
}

// 文字列リテラルを読み込み（クォートなし）
Token lexer_read_string(Lexer* lexer) {
    Token token;
    token.type = TOKEN_STRING;
    token.line = lexer->line;
    token.column = lexer->column;
    
    char buffer[1000];
    int i = 0;
    
    // 英数字、アンダースコアを読み込み（文字列として）
    while (lexer->current_char != '\0' && 
           (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        buffer[i++] = lexer->current_char;
        lexer_advance(lexer);
    }
    buffer[i] = '\0';
    
    token.value = malloc(strlen(buffer) + 1);
    strcpy(token.value, buffer);
    
    return token;
}

// 識別子またはキーワードを読み込み
Token lexer_read_identifier(Lexer* lexer) {
    Token token;
    token.line = lexer->line;
    token.column = lexer->column;
    
    char buffer[100];
    int i = 0;
    
    // 英字、数字、アンダースコアを読み込み
    while (lexer->current_char != '\0' && 
           (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        buffer[i++] = lexer->current_char;
        lexer_advance(lexer);
    }
    buffer[i] = '\0';
    
    // キーワードチェック
    if (strcmp(buffer, "re") == 0) {
        token.type = TOKEN_RE;
    } else if (strcmp(buffer, "func") == 0) {
        token.type = TOKEN_FUNC;
    } else if (strcmp(buffer, "write") == 0) {
        token.type = TOKEN_WRITE;
    } else if (strcmp(buffer, "run") == 0) {
        token.type = TOKEN_RUN;
    } else if (strcmp(buffer, "call") == 0) {
        token.type = TOKEN_CALL;
    } else if (strcmp(buffer, "sunum") == 0) {
        token.type = TOKEN_SUNUM;
    } else if (strcmp(buffer, "py") == 0) {
        token.type = TOKEN_PY;
    } else if (strcmp(buffer, "Window") == 0) {
        token.type = TOKEN_WINDOW;
    } else if (strcmp(buffer, "Button") == 0) {
        token.type = TOKEN_BUTTON;
    } else {
        // キーワードでない場合は文字列として扱う
        token.type = TOKEN_STRING;
    }
    
    token.value = malloc(strlen(buffer) + 1);
    strcpy(token.value, buffer);
    
    return token;
}

// 次のトークンを取得
Token lexer_next_token(Lexer* lexer) {
    while (lexer->current_char != '\0') {
        if (isspace(lexer->current_char)) {
            lexer_skip_whitespace(lexer);
            continue;
        }
        
        // コメント
        if (lexer->current_char == '#') {
            lexer_skip_comment(lexer);
            continue;
        }
        
        // 数値（'で始まる）
        if (lexer->current_char == '\'' && 
            lexer->position + 1 < strlen(lexer->input) && 
            isdigit(lexer->input[lexer->position + 1])) {
            return lexer_read_number(lexer);
        }
        
        // 変数識別子（"で始まる）
        if (lexer->current_char == '"' && 
            lexer->position + 1 < strlen(lexer->input) && 
            (isalpha(lexer->input[lexer->position + 1]) || lexer->input[lexer->position + 1] == '_')) {
            return lexer_read_variable(lexer);
        }
        
        // 識別子・文字列（英字で始まる）
        if (isalpha(lexer->current_char) || lexer->current_char == '_') {
            return lexer_read_identifier(lexer);
        }
        
        // 記号類
        Token token;
        token.line = lexer->line;
        token.column = lexer->column;
        token.value = malloc(3);  // 最大2文字 + null終端
        
        switch (lexer->current_char) {
            case '/':
                lexer_advance(lexer);
                if (lexer->current_char == '/') {
                    lexer_advance(lexer);
                    token.type = TOKEN_BLOCK_END;
                    strcpy(token.value, "//");
                } else {
                    token.type = TOKEN_CMD_END;
                    strcpy(token.value, "/");
                }
                return token;
                
            case '=':
                lexer_advance(lexer);
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    token.type = TOKEN_EQ;
                    strcpy(token.value, "==");
                } else {
                    token.type = TOKEN_ASSIGN;
                    strcpy(token.value, "=");
                }
                return token;
                
            case '!':
                lexer_advance(lexer);
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    token.type = TOKEN_NEQ;
                    strcpy(token.value, "!=");
                } else {
                    token.type = TOKEN_ELSE;
                    strcpy(token.value, "!");
                }
                return token;
                
            case '>':
                lexer_advance(lexer);
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    token.type = TOKEN_GTE;
                    strcpy(token.value, ">=");
                } else {
                    token.type = TOKEN_GT;
                    strcpy(token.value, ">");
                }
                return token;
                
            case '<':
                lexer_advance(lexer);
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    token.type = TOKEN_LTE;
                    strcpy(token.value, "<=");
                } else {
                    token.type = TOKEN_LT;
                    strcpy(token.value, "<");
                }
                return token;
                
            // 単一文字トークン
            case '+':
                token.type = TOKEN_PLUS;
                strcpy(token.value, "+");
                lexer_advance(lexer);
                return token;
            case '-':
                token.type = TOKEN_MINUS;
                strcpy(token.value, "-");
                lexer_advance(lexer);
                return token;
            case '@':
                token.type = TOKEN_AT;
                strcpy(token.value, "@");
                lexer_advance(lexer);
                return token;
            case '\\':
                token.type = TOKEN_BACKSLASH;
                strcpy(token.value, "\\");
                lexer_advance(lexer);
                return token;
            case '%':
                token.type = TOKEN_MOD;
                strcpy(token.value, "%");
                lexer_advance(lexer);
                return token;
            case '&':
                token.type = TOKEN_AMPERSAND;
                strcpy(token.value, "&");
                lexer_advance(lexer);
                return token;
            case '|':
                token.type = TOKEN_PIPE;
                strcpy(token.value, "|");
                lexer_advance(lexer);
                return token;
            case '~':
                token.type = TOKEN_TILDE;
                strcpy(token.value, "~");
                lexer_advance(lexer);
                return token;
            case ';':
                token.type = TOKEN_MULTI_CMD;
                strcpy(token.value, ";");
                lexer_advance(lexer);
                return token;
            case '?':
                token.type = TOKEN_IF;
                strcpy(token.value, "?");
                lexer_advance(lexer);
                return token;
            case '(':
                token.type = TOKEN_LPAREN;
                strcpy(token.value, "(");
                lexer_advance(lexer);
                return token;
            case ')':
                token.type = TOKEN_RPAREN;
                strcpy(token.value, ")");
                lexer_advance(lexer);
                return token;
            case ',':
                token.type = TOKEN_COMMA;
                strcpy(token.value, ",");
                lexer_advance(lexer);
                return token;
            case ':':
                token.type = TOKEN_COLON;
                strcpy(token.value, ":");
                lexer_advance(lexer);
                return token;
            case '.':
                token.type = TOKEN_DOT;
                strcpy(token.value, ".");
                lexer_advance(lexer);
                return token;
            case '^':
                token.type = TOKEN_CARET;
                strcpy(token.value, "^");
                lexer_advance(lexer);
                return token;
            case '\'':
                token.type = TOKEN_QUOTE;
                strcpy(token.value, "'");
                lexer_advance(lexer);
                return token;
                
            default:
                // 未知の文字
                token.type = TOKEN_ERROR;
                token.value[0] = lexer->current_char;
                token.value[1] = '\0';
                lexer_advance(lexer);
                return token;
        }
    }
    
    // EOF
    Token eof_token;
    eof_token.type = TOKEN_EOF;
    eof_token.value = malloc(1);
    eof_token.value[0] = '\0';
    eof_token.line = lexer->line;
    eof_token.column = lexer->column;
    return eof_token;
}

// メイン関数：文字列をトークン化
TokenList tokenize(const char* input) {
    TokenList tokens;
    tokens.tokens = malloc(sizeof(Token) * 100);  // 初期容量100
    tokens.count = 0;
    tokens.capacity = 100;
    
    Lexer* lexer = lexer_create(input);
    
    while (1) {
        Token token = lexer_next_token(lexer);
        
        // 容量を超えた場合は拡張
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

// トークンリストのメモリを解放
void free_tokens(TokenList* tokens) {
    for (int i = 0; i < tokens->count; i++) {
        if (tokens->tokens[i].value) {
            free(tokens->tokens[i].value);
        }
    }
    if (tokens->tokens) {
        free(tokens->tokens);
    }
    tokens->count = 0;
    tokens->capacity = 0;
}