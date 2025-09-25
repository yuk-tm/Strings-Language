#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// 抽象構文木（AST）のノード型（仕様準拠）
typedef enum {
    AST_NUMBER,
    AST_STRING,
    AST_IDENTIFIER,
    AST_BINARY_OP,
    AST_UNARY_OP,               // 単項演算子（~など）
    AST_ASSIGNMENT,             // 基本代入 name = value /
    AST_RE_ASSIGNMENT,          // 再代入 re name value /
    AST_SUNUM_STATEMENT,        // 共有変数 sunum name /
    AST_IF_STATEMENT,           // 条件分岐 condition ? stmt ! stmt /
    AST_COMPOUND_STATEMENT,     // 複合文（;で区切られた複数文）
    AST_FUNCTION_CALL,
    AST_WRITE_STATEMENT,        // write文
    AST_RUN_STATEMENT,          // run文
    AST_CALL_STATEMENT,         // call文（外部言語）
    AST_CATEGORY_DEFINITION     // カテゴリ定義（将来用）
} ASTNodeType;

// ASTノード
typedef struct ASTNode {
    ASTNodeType type;
    union {
        struct {
            double value;
        } number;
        
        struct {
            char* value;
        } string;
        
        struct {
            char* name;
        } identifier;
        
        struct {
            TokenType operator;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        
        struct {
            TokenType operator;
            struct ASTNode* operand;
        } unary_op;
        
        struct {
            char* variable;
            struct ASTNode* value;
        } assignment;
        
        struct {
            char* variable;
            struct ASTNode* value;
        } re_assignment;
        
        struct {
            char* variable;
        } sunum_statement;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_stmt;
            struct ASTNode* else_stmt;
        } if_statement;
        
        struct {
            struct ASTNode** statements;
            int count;
        } compound_statement;
        
        struct {
            char* function_name;
            struct ASTNode** arguments;
            int arg_count;
        } function_call;
        
        struct {
            struct ASTNode* expression;
        } write_statement;
        
        struct {
            char* category_name;
        } run_statement;
        
        struct {
            char* language;
            char* code;  // 外部言語コード
        } call_statement;
    } data;
} ASTNode;

// パーサー構造体
typedef struct {
    TokenList tokens;
    int position;
    Token current_token;
} Parser;

// 関数宣言
Parser* parser_create(TokenList tokens);
void parser_free(Parser* parser);
ASTNode* parse(Parser* parser);
void ast_free(ASTNode* node);

// 内部関数宣言
void parser_advance(Parser* parser);
int parser_expect(Parser* parser, TokenType expected);
ASTNode* ast_create_node(ASTNodeType type);
ASTNode* parse_primary(Parser* parser);
ASTNode* parse_term(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_comparison(Parser* parser);
ASTNode* parse_logical(Parser* parser);
ASTNode* parse_write_statement(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
ASTNode* parse_re_statement(Parser* parser);
ASTNode* parse_sunum_statement(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_run_statement(Parser* parser);
ASTNode* parse_call_statement(Parser* parser);
ASTNode* parse_statement(Parser* parser);

#endif