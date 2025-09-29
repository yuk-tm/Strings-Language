#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

ASTNode* parse_expression(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_re_assignment_statement(Parser* parser);


Parser* parser_create(TokenList tokens) {
    Parser* parser = malloc(sizeof(Parser));
    parser->tokens = tokens;
    parser->position = 0;
    parser->current_token = tokens.count > 0 ? tokens.tokens[0] : (Token){TOKEN_EOF, NULL, 0, 0};
    return parser;
}

void parser_free(Parser* parser) {
    if (parser) {
        free(parser);
    }
}

void parser_advance(Parser* parser) {
    parser->position++;
    if (parser->position < parser->tokens.count) {
        parser->current_token = parser->tokens.tokens[parser->position];
    } else {
        parser->current_token.type = TOKEN_EOF;
    }
}

int parser_expect(Parser* parser, TokenType expected) {
    if (parser->current_token.type == expected) {
        parser_advance(parser);
        return 1;
    } else {
        printf("Parse error at line %d, column %d: Expected %s, got %s\n", 
               parser->current_token.line, parser->current_token.column,
               token_to_string(expected), token_to_string(parser->current_token.type));
        return 0;
    }
}

ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    return node;
}

void ast_free(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_NUMBER:
            break;
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
        case AST_UNARY_OP:
            ast_free(node->data.unary_op.operand);
            break;
        case AST_ASSIGNMENT:
        case AST_RE_ASSIGNMENT:
        case AST_SUNUM_STATEMENT:
            if (node->data.assignment.variable) free(node->data.assignment.variable);
            ast_free(node->data.assignment.expression);
            break;
        case AST_IF_STATEMENT:
            ast_free(node->data.if_statement.condition);
            ast_free(node->data.if_statement.then_stmt);
            ast_free(node->data.if_statement.else_stmt);
            break;
        case AST_COMPOUND_STATEMENT:
            for (int i = 0; i < node->data.compound_statement.statement_count; i++) {
                ast_free(node->data.compound_statement.statements[i]);
            }
            if (node->data.compound_statement.statements) free(node->data.compound_statement.statements);
            break;
        case AST_WRITE_STATEMENT:
            ast_free(node->data.write_statement.expression);
            break;
        case AST_RUN_STATEMENT:
            if (node->data.run_statement.category_name) free(node->data.run_statement.category_name);
            break;
        case AST_CALL_STATEMENT:
            if (node->data.call_statement.language) free(node->data.call_statement.language);
            if (node->data.call_statement.code) free(node->data.call_statement.code);
            break;
        case AST_FUNCTION_CALL:
            if (node->data.function_call.function_name) free(node->data.function_call.function_name);
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                ast_free(node->data.function_call.arguments[i]);
            }
            if (node->data.function_call.arguments) free(node->data.function_call.arguments);
            break;
        case AST_CATEGORY_DEFINITION:
            if (node->data.category_definition.name) free(node->data.category_definition.name);
            for (int i = 0; i < node->data.category_definition.statement_count; i++) {
                ast_free(node->data.category_definition.statements[i]);
            }
            if (node->data.category_definition.statements) free(node->data.category_definition.statements);
            break;
    }
    free(node);
}

ASTNode* parse_primary(Parser* parser) {
    ASTNode* node = NULL;
    
    switch (parser->current_token.type) {
        case TOKEN_NUMBER:
            node = ast_create_node(AST_NUMBER);
            node->data.number.value = atof(parser->current_token.value);
            parser_advance(parser);
            break;
        case TOKEN_STRING:
            node = ast_create_node(AST_STRING);
            node->data.string.value = strdup(parser->current_token.value);
            parser_advance(parser);
            break;
        case TOKEN_IDENTIFIER:
            node = ast_create_node(AST_IDENTIFIER);
            node->data.identifier.name = strdup(parser->current_token.value);
            parser_advance(parser);
            break;
        case TOKEN_LPAREN:
            parser_advance(parser);
            node = parse_expression(parser);
            if (!parser_expect(parser, TOKEN_RPAREN)) {
                ast_free(node);
                return NULL;
            }
            break;
        default:
            printf("Parse error at line %d, column %d: Expected expression start, got %s\n", 
                   parser->current_token.line, parser->current_token.column,
                   token_to_string(parser->current_token.type));
            return NULL;
    }
    return node;
}

ASTNode* parse_unary(Parser* parser) {
    TokenType op_type = parser->current_token.type;
    
    if (op_type == TOKEN_PLUS || op_type == TOKEN_MINUS || op_type == TOKEN_TILDE) {
        parser_advance(parser);
        ASTNode* operand = parse_unary(parser);
        if (!operand) return NULL;
        
        ASTNode* node = ast_create_node(AST_UNARY_OP);
        node->data.unary_op.operator = op_type;
        node->data.unary_op.operand = operand;
        return node;
    }
    
    return parse_primary(parser);
}

ASTNode* parse_term(Parser* parser) {
    ASTNode* node = parse_unary(parser);
    if (!node) return NULL;
    
    while (parser->current_token.type == TOKEN_AT || 
           parser->current_token.type == TOKEN_YEN || 
           parser->current_token.type == TOKEN_BACKSLASH || 
           parser->current_token.type == TOKEN_MOD) {
        
        TokenType op_type = parser->current_token.type;
        parser_advance(parser);
        ASTNode* right = parse_unary(parser);
        if (!right) {
            ast_free(node);
            return NULL;
        }
        
        ASTNode* new_node = ast_create_node(AST_BINARY_OP);
        new_node->data.binary_op.operator = op_type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    
    return node;
}

ASTNode* parse_additive(Parser* parser) {
    ASTNode* node = parse_term(parser);
    if (!node) return NULL;
    
    while (parser->current_token.type == TOKEN_PLUS || 
           parser->current_token.type == TOKEN_MINUS) {
        
        TokenType op_type = parser->current_token.type;
        parser_advance(parser);
        ASTNode* right = parse_term(parser);
        if (!right) {
            ast_free(node);
            return NULL;
        }
        
        ASTNode* new_node = ast_create_node(AST_BINARY_OP);
        new_node->data.binary_op.operator = op_type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    
    return node;
}

ASTNode* parse_comparison(Parser* parser) {
    ASTNode* node = parse_additive(parser);
    if (!node) return NULL;

    while (parser->current_token.type == TOKEN_GT ||
           parser->current_token.type == TOKEN_LT ||
           parser->current_token.type == TOKEN_GTE ||
           parser->current_token.type == TOKEN_LTE ||
           parser->current_token.type == TOKEN_EQ ||
           parser->current_token.type == TOKEN_NEQ) {
        
        TokenType op_type = parser->current_token.type;
        parser_advance(parser);
        ASTNode* right = parse_additive(parser);
        if (!right) {
            ast_free(node);
            return NULL;
        }
        
        ASTNode* new_node = ast_create_node(AST_BINARY_OP);
        new_node->data.binary_op.operator = op_type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }

    return node;
}

ASTNode* parse_logical(Parser* parser) {
    ASTNode* node = parse_comparison(parser);
    if (!node) return NULL;

    while (parser->current_token.type == TOKEN_AMPERSAND ||
           parser->current_token.type == TOKEN_PIPE) {
        
        TokenType op_type = parser->current_token.type;
        parser_advance(parser);
        ASTNode* right = parse_comparison(parser);
        if (!right) {
            ast_free(node);
            return NULL;
        }
        
        ASTNode* new_node = ast_create_node(AST_BINARY_OP);
        new_node->data.binary_op.operator = op_type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }

    return node;
}

ASTNode* parse_expression(Parser* parser) {
    return parse_logical(parser);
}

ASTNode* parse_write_statement(Parser* parser) {
    if (!parser_expect(parser, TOKEN_WRITE)) return NULL;
    
    ASTNode* expression = parse_expression(parser);
    if (!expression) return NULL;
    
    ASTNode* node = ast_create_node(AST_WRITE_STATEMENT);
    node->data.write_statement.expression = expression;
    return node;
}

ASTNode* parse_assignment(Parser* parser) {
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        printf("Parse error: Expected identifier for assignment\n");
        return NULL;
    }
    
    char* var_name = strdup(parser->current_token.value);
    parser_advance(parser);
    
    if (!parser_expect(parser, TOKEN_ASSIGN)) {
        free(var_name);
        return NULL;
    }
    
    ASTNode* expression = parse_expression(parser);
    if (!expression) {
        free(var_name);
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_ASSIGNMENT);
    node->data.assignment.variable = var_name;
    node->data.assignment.expression = expression;
    return node;
}

ASTNode* parse_re_assignment_statement(Parser* parser) {
    if (!parser_expect(parser, TOKEN_RE)) return NULL;
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        printf("Parse error: Expected identifier for re-assignment\n");
        return NULL;
    }
    
    char* var_name = strdup(parser->current_token.value);
    parser_advance(parser);
    
    ASTNode* expression = parse_expression(parser);
    if (!expression) {
        free(var_name);
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_RE_ASSIGNMENT);
    node->data.assignment.variable = var_name;
    node->data.assignment.expression = expression;
    return node;
}

ASTNode* parse_sunum_statement(Parser* parser) {
    if (!parser_expect(parser, TOKEN_SUNUM)) return NULL;
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        printf("Parse error: Expected identifier for sunum statement\n");
        return NULL;
    }
    
    char* var_name = strdup(parser->current_token.value);
    parser_advance(parser);
    
    ASTNode* node = ast_create_node(AST_SUNUM_STATEMENT);
    node->data.assignment.variable = var_name;
    node->data.assignment.expression = NULL;
    return node;
}

ASTNode* parse_run_statement(Parser* parser) {
    if (!parser_expect(parser, TOKEN_RUN)) return NULL;
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        printf("Parse error: Expected category name for run statement\n");
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_RUN_STATEMENT);
    node->data.run_statement.category_name = strdup(parser->current_token.value);
    parser_advance(parser);
    return node;
}

ASTNode* parse_call_statement(Parser* parser) {
    if (!parser_expect(parser, TOKEN_CALL)) return NULL;
    
    if (parser->current_token.type != TOKEN_PY && 
        parser->current_token.type != TOKEN_IDENTIFIER) {
        printf("Parse error: Expected language identifier for call statement\n");
        return NULL;
    }
    
    char* language = strdup(parser->current_token.value);
    parser_advance(parser);
    
    ASTNode* code_expr = parse_expression(parser);
    if (!code_expr || code_expr->type != AST_STRING) {
        printf("Parse error: Expected string expression (code) for call statement\n");
        free(language);
        ast_free(code_expr);
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_CALL_STATEMENT);
    node->data.call_statement.language = language;
    node->data.call_statement.code = code_expr->data.string.value;
    code_expr->data.string.value = NULL;
    ast_free(code_expr);
    
    return node;
}

ASTNode* parse_category_definition(Parser* parser) {
    if (!parser_expect(parser, TOKEN_FUNC)) return NULL;

    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        printf("Parse error: Expected category name after 'func'\n");
        return NULL;
    }

    char* name = strdup(parser->current_token.value);
    parser_advance(parser);

    if (!parser_expect(parser, TOKEN_LPAREN)) {
        free(name);
        return NULL;
    }

    if (!parser_expect(parser, TOKEN_RPAREN)) {
        free(name);
        return NULL;
    }

    ASTNode** statements = malloc(sizeof(ASTNode*) * 10);
    int count = 0;
    int capacity = 10;
    
    while (parser->current_token.type != TOKEN_BLOCK_END && parser->current_token.type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            if (count >= capacity) {
                capacity *= 2;
                statements = realloc(statements, sizeof(ASTNode*) * capacity);
            }
            statements[count++] = stmt;
        } else {
            for (int i = 0; i < count; i++) ast_free(statements[i]);
            free(statements);
            free(name);
            return NULL;
        }
    }

    if (!parser_expect(parser, TOKEN_BLOCK_END)) {
        for (int i = 0; i < count; i++) ast_free(statements[i]);
        free(statements);
        free(name);
        return NULL;
    }

    ASTNode* node = ast_create_node(AST_CATEGORY_DEFINITION);
    node->data.category_definition.name = name;
    node->data.category_definition.statements = statements;
    node->data.category_definition.statement_count = count;
    return node;
}

ASTNode* parse_statement_end(Parser* parser, ASTNode* node) {
    if (parser->current_token.type == TOKEN_CMD_END) {
        parser_advance(parser);
        return node;
    } else if (parser->current_token.type == TOKEN_MULTI_CMD) {
        parser_advance(parser);
        ASTNode* next_stmt = parse_statement(parser);
        
        if (!next_stmt) {
            ast_free(node);
            return NULL;
        }
        
        ASTNode* compound = ast_create_node(AST_COMPOUND_STATEMENT);
        compound->data.compound_statement.statements = malloc(sizeof(ASTNode*) * 2);
        compound->data.compound_statement.statements[0] = node;
        compound->data.compound_statement.statements[1] = next_stmt;
        compound->data.compound_statement.statement_count = 2;
        
        if (next_stmt->type == AST_COMPOUND_STATEMENT) {
            int old_count = compound->data.compound_statement.statement_count;
            int new_count = old_count + next_stmt->data.compound_statement.statement_count - 1;
            compound->data.compound_statement.statements = realloc(compound->data.compound_statement.statements, sizeof(ASTNode*) * new_count);
            
            for (int i = 0; i < next_stmt->data.compound_statement.statement_count; i++) {
                compound->data.compound_statement.statements[old_count + i - 1] = next_stmt->data.compound_statement.statements[i];
            }
            compound->data.compound_statement.statement_count = new_count;
            
            free(next_stmt->data.compound_statement.statements);
            next_stmt->data.compound_statement.statements = NULL;
            ast_free(next_stmt);
        }
        
        return compound;
    } else {
        printf("Parse error: Expected / or ; after statement, got %s\n", token_to_string(parser->current_token.type));
        ast_free(node);
        return NULL;
    }
}

ASTNode* parse_if_statement_after_condition(Parser* parser, ASTNode* condition) {
    if (!parser_expect(parser, TOKEN_IF)) {
        ast_free(condition);
        return NULL;
    }

    ASTNode* then_stmt = parse_statement(parser);
    if (!then_stmt) {
        ast_free(condition);
        return NULL;
    }
    
    ASTNode* else_stmt = NULL;
    if (parser->current_token.type == TOKEN_ELSE) {
        parser_advance(parser);
        else_stmt = parse_statement(parser);
        if (!else_stmt) {
            ast_free(condition);
            ast_free(then_stmt);
            return NULL;
        }
    }

    ASTNode* if_node = ast_create_node(AST_IF_STATEMENT);
    if_node->data.if_statement.condition = condition;
    if_node->data.if_statement.then_stmt = then_stmt;
    if_node->data.if_statement.else_stmt = else_stmt;
    
    return if_node;
}

ASTNode* parse_if_statement(Parser* parser) {
    ASTNode* condition = parse_expression(parser);
    if (!condition) return NULL;
    
    return parse_if_statement_after_condition(parser, condition);
}

ASTNode* parse_statement(Parser* parser) {
    ASTNode* node = NULL;
    
    if (parser->current_token.type == TOKEN_FUNC) {
        node = parse_category_definition(parser);
        if (node) return node;
        return NULL;
    }
    
    switch (parser->current_token.type) {
        case TOKEN_WRITE:
            node = parse_write_statement(parser);
            break;
        case TOKEN_RE:
            node = parse_re_assignment_statement(parser);
            break;
        case TOKEN_SUNUM:
            node = parse_sunum_statement(parser);
            break;
        case TOKEN_RUN:
            node = parse_run_statement(parser);
            break;
        case TOKEN_CALL:
            node = parse_call_statement(parser);
            break;
        case TOKEN_IDENTIFIER:
            if (parser->position + 1 < parser->tokens.count && parser->tokens.tokens[parser->position + 1].type == TOKEN_ASSIGN) {
                node = parse_assignment(parser);
            } else {
                 return parse_if_statement(parser);
            }
            
            if (node) {
                if (node->type == AST_ASSIGNMENT) {
                    return parse_statement_end(parser, node);
                } else if (parser->current_token.type == TOKEN_CMD_END) {
                    parser_advance(parser);
                    return node;
                } else if (parser->current_token.type == TOKEN_MULTI_CMD) {
                    return parse_if_statement_after_condition(parser, node);
                } else {
                    printf("Parse error: Expected / or ; after assignment\n");
                    ast_free(node);
                    return NULL;
                }
            } else {
                 return parse_if_statement(parser);
            }

        case TOKEN_NUMBER:
        case TOKEN_LPAREN:
             return parse_if_statement(parser);

        default:
            if (parser->current_token.type == TOKEN_CMD_END) {
                return NULL;
            }
            printf("Parse error: Unexpected statement start %s\n", token_to_string(parser->current_token.type));
            return NULL;
    }
    
    if (node) {
        return parse_statement_end(parser, node);
    }
    
    return NULL;
}

ASTNode* parse(Parser* parser) {
    if (parser->current_token.type == TOKEN_EOF) {
        return NULL;
    }
    
    ASTNode* result_node = parse_statement(parser);
    
    if (parser->current_token.type == TOKEN_CMD_END) {
        parser_advance(parser); 
    }
    
    if (parser->current_token.type != TOKEN_EOF && parser->current_token.type != TOKEN_BLOCK_END) {
        if (result_node) ast_free(result_node);
        printf("Parse error: Expected EOF or // at end of parsing, got %s\n", token_to_string(parser->current_token.type));
        return NULL;
    }
    
    return result_node;
}