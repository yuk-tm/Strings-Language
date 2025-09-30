#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// ASTノードタイプ
typedef enum {
    AST_NUMBER,
    AST_STRING,
    AST_IDENTIFIER,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_ASSIGNMENT,
    AST_RE_ASSIGNMENT,
    AST_SUNUM_STATEMENT,
    AST_IF_STATEMENT,
    AST_COMPOUND_STATEMENT,
    AST_FUNCTION_CALL,
    AST_WRITE_STATEMENT,
    AST_NUM_WRITE_STATEMENT,
    AST_RUN_STATEMENT,
    AST_CALL_STATEMENT,
    AST_CATEGORY_DEFINITION
} ASTNodeType;

// ASTノード構造体
typedef struct ASTNode {
    ASTNodeType type;
    union {
        struct { double value; } number;
        struct { char* value; } string;
        struct { char* name; } identifier;
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
            struct ASTNode* expression;
        } assignment;
        struct { char* variable_name; } num_write_statement;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_stmt;
            struct ASTNode* else_stmt;
        } if_statement;
        struct {
            struct ASTNode** statements;
            int statement_count;
        } compound_statement;
        struct {
            char* name;
            struct ASTNode** statements;
            int statement_count;
        } category_definition;
        struct {
            char* function_name;
            struct ASTNode** arguments;
            int arg_count;
        } function_call;
        struct { struct ASTNode* expression; } write_statement;
        struct { char* category_name; } run_statement;
        struct { char* language; char* code; } call_statement;
    } data;
} ASTNode;

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

void parser_advance(Parser* parser);
int parser_expect(Parser* parser, TokenType expected);
ASTNode* ast_create_node(ASTNodeType type);
ASTNode* parse_primary(Parser* parser);
ASTNode* parse_term(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_comparison(Parser* parser);
ASTNode* parse_logical(Parser* parser);
ASTNode* parse_write_statement(Parser* parser);
ASTNode* parse_num_write_statement(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
ASTNode* parse_re_assignment_statement(Parser* parser);
ASTNode* parse_sunum_statement(Parser* parser);
ASTNode* parse_run_statement(Parser* parser);
ASTNode* parse_call_statement(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_if_statement_after_condition(Parser* parser, ASTNode* condition);
ASTNode* parse_category_definition(Parser* parser);

#endif