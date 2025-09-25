#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// 前方宣言
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_statement(Parser* parser);

// パーサーを作成
Parser* parser_create(TokenList tokens) {
    Parser* parser = malloc(sizeof(Parser));
    parser->tokens = tokens;
    parser->position = 0;
    parser->current_token = tokens.count > 0 ? tokens.tokens[0] : (Token){TOKEN_EOF, NULL, 0, 0};
    return parser;
}

// パーサーを解放
void parser_free(Parser* parser) {
    if (parser) {
        free(parser);
    }
}

// 次のトークンに進む
void parser_advance(Parser* parser) {
    parser->position++;
    if (parser->position < parser->tokens.count) {
        parser->current_token = parser->tokens.tokens[parser->position];
    } else {
        parser->current_token.type = TOKEN_EOF;
    }
}

// 指定されたトークン型を期待して消費
int parser_expect(Parser* parser, TokenType expected) {
    if (parser->current_token.type == expected) {
        parser_advance(parser);
        return 1;  // 成功
    }
    printf("Parse error: Expected %s but found %s\n", 
           token_to_string(expected), token_to_string(parser->current_token.type));
    return 0;  // 失敗
}

// ASTノードを作成
ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    return node;
}

// ASTノードを解放
void ast_free(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_STRING:
            if (node->data.string.value) free(node->data.string.value);
            break;
        case AST_IDENTIFIER:
            if (node->data.identifier.name) free(node->data.identifier.name);
            break;
        case AST_BINARY_OP:
            ast_free(node->data.binary_op.left);
            ast_free(node->data.binary_op.right);
            break;
        case AST_ASSIGNMENT:
            if (node->data.assignment.variable) free(node->data.assignment.variable);
            ast_free(node->data.assignment.value);
            break;
        case AST_IF_STATEMENT:
            ast_free(node->data.if_statement.condition);
            ast_free(node->data.if_statement.then_stmt);
            ast_free(node->data.if_statement.else_stmt);
            break;
        case AST_FUNCTION_CALL:
            if (node->data.function_call.function_name) free(node->data.function_call.function_name);
            if (node->data.function_call.arguments) {
                for (int i = 0; i < node->data.function_call.arg_count; i++) {
                    ast_free(node->data.function_call.arguments[i]);
                }
                free(node->data.function_call.arguments);
            }
            break;
        case AST_WRITE_STATEMENT:
            ast_free(node->data.write_statement.expression);
            break;
        default:
            break;
    }
    free(node);
}

// 基本要素をパース（数値、文字列、識別子、括弧）
ASTNode* parse_primary(Parser* parser) {
    ASTNode* node;
    
    switch (parser->current_token.type) {
        case TOKEN_NUMBER:
            // '5 形式の数値
            node = ast_create_node(AST_NUMBER);
            node->data.number.value = atof(parser->current_token.value);
            parser_advance(parser);
            return node;
            
        case TOKEN_STRING:
            // クォートなし文字列
            node = ast_create_node(AST_STRING);
            node->data.string.value = malloc(strlen(parser->current_token.value) + 1);
            strcpy(node->data.string.value, parser->current_token.value);
            parser_advance(parser);
            return node;
            
        case TOKEN_IDENTIFIER:
            // 変数として扱う（後で文脈に応じて変更）
            node = ast_create_node(AST_IDENTIFIER);
            node->data.identifier.name = malloc(strlen(parser->current_token.value) + 1);
            strcpy(node->data.identifier.name, parser->current_token.value);
            parser_advance(parser);
            return node;
            
        case TOKEN_LPAREN:
            parser_advance(parser);  // '(' をスキップ
            node = parse_expression(parser);
            parser_expect(parser, TOKEN_RPAREN);  // ')' を期待
            return node;
            
        case TOKEN_TILDE:
            // 論理否定 ~
            parser_advance(parser);
            node = ast_create_node(AST_UNARY_OP);
            node->data.unary_op.operator = TOKEN_TILDE;
            node->data.unary_op.operand = parse_primary(parser);
            return node;
            
        default:
            printf("Parse error: Unexpected token %s\n", token_to_string(parser->current_token.type));
            return NULL;
    }
}

// 乗算・除算レベルの式をパース
ASTNode* parse_term(Parser* parser) {
    ASTNode* node = parse_primary(parser);
    
    while (parser->current_token.type == TOKEN_AT ||      // @
           parser->current_token.type == TOKEN_BACKSLASH ||  // \ 
           parser->current_token.type == TOKEN_YEN ||      // ¥
           parser->current_token.type == TOKEN_MOD) {      // %
        
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        
        ASTNode* right = parse_primary(parser);
        
        ASTNode* binary_node = ast_create_node(AST_BINARY_OP);
        binary_node->data.binary_op.operator = op;
        binary_node->data.binary_op.left = node;
        binary_node->data.binary_op.right = right;
        
        node = binary_node;
    }
    
    return node;
}

// 加算・減算レベルの式をパース
ASTNode* parse_expression(Parser* parser) {
    ASTNode* node = parse_term(parser);
    
    while (parser->current_token.type == TOKEN_PLUS || parser->current_token.type == TOKEN_MINUS) {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        
        ASTNode* right = parse_term(parser);
        
        ASTNode* binary_node = ast_create_node(AST_BINARY_OP);
        binary_node->data.binary_op.operator = op;
        binary_node->data.binary_op.left = node;
        binary_node->data.binary_op.right = right;
        
        node = binary_node;
    }
    
    return node;
}

// 比較式をパース
ASTNode* parse_comparison(Parser* parser) {
    ASTNode* node = parse_expression(parser);
    
    if (parser->current_token.type == TOKEN_GT || 
        parser->current_token.type == TOKEN_LT || 
        parser->current_token.type == TOKEN_GTE || 
        parser->current_token.type == TOKEN_LTE || 
        parser->current_token.type == TOKEN_EQ || 
        parser->current_token.type == TOKEN_NEQ) {
        
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        
        ASTNode* right = parse_expression(parser);
        
        ASTNode* binary_node = ast_create_node(AST_BINARY_OP);
        binary_node->data.binary_op.operator = op;
        binary_node->data.binary_op.left = node;
        binary_node->data.binary_op.right = right;
        
        return binary_node;
    }
    
    return node;
}

// 論理式をパース
ASTNode* parse_logical(Parser* parser) {
    ASTNode* node = parse_comparison(parser);
    
    while (parser->current_token.type == TOKEN_AMPERSAND || // &
           parser->current_token.type == TOKEN_PIPE) {      // |
        
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        
        ASTNode* right = parse_comparison(parser);
        
        ASTNode* binary_node = ast_create_node(AST_BINARY_OP);
        binary_node->data.binary_op.operator = op;
        binary_node->data.binary_op.left = node;
        binary_node->data.binary_op.right = right;
        
        node = binary_node;
    }
    
    return node;
}

// write文をパース
ASTNode* parse_write_statement(Parser* parser) {
    parser_advance(parser);  // 'write' をスキップ
    
    ASTNode* node = ast_create_node(AST_WRITE_STATEMENT);
    node->data.write_statement.expression = parse_expression(parser);
    
    return node;
}

// 代入文をパース
ASTNode* parse_assignment(Parser* parser) {
    char* var_name = malloc(strlen(parser->current_token.value) + 1);
    strcpy(var_name, parser->current_token.value);
    
    parser_advance(parser);  // 変数名をスキップ
    parser_expect(parser, TOKEN_ASSIGN);  // '=' を期待
    
    ASTNode* node = ast_create_node(AST_ASSIGNMENT);
    node->data.assignment.variable = var_name;
    node->data.assignment.value = parse_expression(parser);
    
    return node;
}

// re文（再代入）をパース
ASTNode* parse_re_statement(Parser* parser) {
    parser_advance(parser);  // 're' をスキップ
    
    if (parser->current_token.type != TOKEN_STRING) {
        printf("Parse error: Expected variable name after 're'\n");
        return NULL;
    }
    
    char* var_name = malloc(strlen(parser->current_token.value) + 1);
    strcpy(var_name, parser->current_token.value);
    parser_advance(parser);
    
    ASTNode* node = ast_create_node(AST_RE_ASSIGNMENT);
    node->data.re_assignment.variable = var_name;
    node->data.re_assignment.value = parse_expression(parser);
    
    return node;
}

// sunum文（共有変数）をパース
ASTNode* parse_sunum_statement(Parser* parser) {
    parser_advance(parser);  // 'sunum' をスキップ
    
    if (parser->current_token.type != TOKEN_STRING) {
        printf("Parse error: Expected variable name after 'sunum'\n");
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_SUNUM_STATEMENT);
    node->data.sunum_statement.variable = malloc(strlen(parser->current_token.value) + 1);
    strcpy(node->data.sunum_statement.variable, parser->current_token.value);
    parser_advance(parser);
    
    return node;
}

// if文をパース（仕様準拠: condition ? stmt; stmt ! stmt; stmt /）
ASTNode* parse_if_statement(Parser* parser) {
    ASTNode* condition = parse_logical(parser);
    
    parser_expect(parser, TOKEN_IF);  // '?' を期待
    
    // then部分の文をパース（;区切りで複数可能）
    ASTNode* then_stmt = parse_statement(parser);
    
    // 複数命令がある場合
    while (parser->current_token.type == TOKEN_MULTI_CMD) {
        parser_advance(parser);  // ';' をスキップ
        ASTNode* next_stmt = parse_statement(parser);
        
        // 複数文を連結するためのノード（簡易実装）
        ASTNode* compound = ast_create_node(AST_COMPOUND_STATEMENT);
        compound->data.compound_statement.statements = malloc(sizeof(ASTNode*) * 2);
        compound->data.compound_statement.statements[0] = then_stmt;
        compound->data.compound_statement.statements[1] = next_stmt;
        compound->data.compound_statement.count = 2;
        then_stmt = compound;
    }
    
    ASTNode* else_stmt = NULL;
    if (parser->current_token.type == TOKEN_ELSE) {
        parser_advance(parser);  // '!' をスキップ
        else_stmt = parse_statement(parser);
        
        // else部分でも複数命令対応
        while (parser->current_token.type == TOKEN_MULTI_CMD) {
            parser_advance(parser);  // ';' をスキップ
            ASTNode* next_stmt = parse_statement(parser);
            
            ASTNode* compound = ast_create_node(AST_COMPOUND_STATEMENT);
            compound->data.compound_statement.statements = malloc(sizeof(ASTNode*) * 2);
            compound->data.compound_statement.statements[0] = else_stmt;
            compound->data.compound_statement.statements[1] = next_stmt;
            compound->data.compound_statement.count = 2;
            else_stmt = compound;
        }
    }
    
    parser_expect(parser, TOKEN_CMD_END);  // '/' を期待
    
    ASTNode* node = ast_create_node(AST_IF_STATEMENT);
    node->data.if_statement.condition = condition;
    node->data.if_statement.then_stmt = then_stmt;
    node->data.if_statement.else_stmt = else_stmt;
    
    return node;
}

// run文をパース
ASTNode* parse_run_statement(Parser* parser) {
    parser_advance(parser);  // 'run' をスキップ
    
    if (parser->current_token.type != TOKEN_STRING) {
        printf("Parse error: Expected category name after 'run'\n");
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_RUN_STATEMENT);
    node->data.run_statement.category_name = malloc(strlen(parser->current_token.value) + 1);
    strcpy(node->data.run_statement.category_name, parser->current_token.value);
    parser_advance(parser);
    
    return node;
}

// call文をパース
ASTNode* parse_call_statement(Parser* parser) {
    parser_advance(parser);  // 'call' をスキップ
    
    if (parser->current_token.type != TOKEN_STRING) {
        printf("Parse error: Expected language name after 'call'\n");
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_CALL_STATEMENT);
    node->data.call_statement.language = malloc(strlen(parser->current_token.value) + 1);
    strcpy(node->data.call_statement.language, parser->current_token.value);
    parser_advance(parser);
    
    // TODO: 外部言語コードの解析を実装
    
    return node;
}

// 1つの文をパース
ASTNode* parse_statement(Parser* parser) {
    switch (parser->current_token.type) {
        case TOKEN_WRITE:
            return parse_write_statement(parser);
            
        case TOKEN_RE:
            return parse_re_statement(parser);
            
        case TOKEN_SUNUM:
            return parse_sunum_statement(parser);
            
        case TOKEN_RUN:
            return parse_run_statement(parser);
            
        case TOKEN_CALL:
            return parse_call_statement(parser);
            
        case TOKEN_STRING:
            // 次のトークンが '=' なら代入文
            if (parser->position + 1 < parser->tokens.count && 
                parser->tokens.tokens[parser->position + 1].type == TOKEN_ASSIGN) {
                return parse_assignment(parser);
            }
            // そうでなければ条件分岐の可能性
            return parse_if_statement(parser);
            
        case TOKEN_NUMBER:
            // 数値で始まる場合は条件分岐
            return parse_if_statement(parser);
            
        default:
            printf("Parse error: Unexpected statement start %s\n", token_to_string(parser->current_token.type));
            return NULL;
    }
}

// メイン解析関数
ASTNode* parse(Parser* parser) {
    if (parser->current_token.type == TOKEN_EOF) {
        return NULL;
    }
    
    ASTNode* stmt = parse_statement(parser);
    
    // 文の終了を確認
    if (parser->current_token.type == TOKEN_CMD_END) {
        parser_advance(parser);
    }
    
    return stmt;
}