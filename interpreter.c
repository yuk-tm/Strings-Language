#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter.h"

EvalResult create_number_result(double value) {
    EvalResult result;
    result.type = RESULT_NUMBER;
    result.value.number = value;
    return result;
}

EvalResult create_string_result(const char* value) {
    EvalResult result;
    result.type = RESULT_STRING;
    result.value.string = strdup(value);
    return result;
}

void variable_table_expand(VariableTable* table) {
    table->capacity *= 2;
    table->variables = realloc(table->variables, sizeof(Variable) * table->capacity);
}

Variable* find_variable(VariableTable* table, const char* name) {
    for (int i = 0; i < table->count; i++)
        if (strcmp(table->variables[i].name, name) == 0)
            return &table->variables[i];
    return NULL;
}

void set_variable_internal(VariableTable* table, const char* name, EvalResult result, int is_shared) {
    Variable* var = find_variable(table, name);
    if (var) {
        if (var->type == VAR_STRING && var->value.string != NULL) free(var->value.string);
        switch (result.type) {
            case RESULT_NUMBER: var->type = VAR_NUMBER; var->value.number = result.value.number; break;
            case RESULT_STRING: var->type = VAR_STRING; var->value.string = strdup(result.value.string); break;
        }
    } else {
        if (table->count >= table->capacity) variable_table_expand(table);
        var = &table->variables[table->count++];
        var->name = strdup(name);
        var->is_shared = is_shared;
        switch (result.type) {
            case RESULT_NUMBER: var->type = VAR_NUMBER; var->value.number = result.value.number; break;
            case RESULT_STRING: var->type = VAR_STRING; var->value.string = strdup(result.value.string); break;
        }
    }
}

void set_variable(Interpreter* interpreter, const char* name, EvalResult result, int is_shared) {
    if (is_shared) set_variable_internal(&interpreter->shared_variables, name, result, is_shared);
    else set_variable_internal(&interpreter->variables, name, result, is_shared);
}

Variable* get_variable(Interpreter* interpreter, const char* name) {
    Variable* var = find_variable(&interpreter->variables, name);
    if (!var) var = find_variable(&interpreter->shared_variables, name);
    return var;
}

void set_shared_variable(Interpreter* interpreter, const char* name) {
    Variable* local_var = find_variable(&interpreter->variables, name);
    if (local_var) {
        if (local_var->type == VAR_STRING) {
            EvalResult result = create_string_result(local_var->value.string);
            set_variable_internal(&interpreter->shared_variables, name, result, 1);
            free(result.value.string);
        } else {
            EvalResult result = create_number_result(local_var->value.number);
            set_variable_internal(&interpreter->shared_variables, name, result, 1);
        }
    }
}

Interpreter* interpreter_create() {
    Interpreter* interpreter = malloc(sizeof(Interpreter));
    interpreter->variables.variables = malloc(sizeof(Variable) * 10);
    interpreter->variables.count = 0;
    interpreter->variables.capacity = 10;
    interpreter->shared_variables.variables = malloc(sizeof(Variable) * 10);
    interpreter->shared_variables.count = 0;
    interpreter->shared_variables.capacity = 10;
    interpreter->categories = NULL;
    return interpreter;
}

void interpreter_free(Interpreter* interpreter) {
    if (!interpreter) return;
    for (int i = 0; i < interpreter->variables.count; i++) {
        free(interpreter->variables.variables[i].name);
        if (interpreter->variables.variables[i].type == VAR_STRING)
            free(interpreter->variables.variables[i].value.string);
    }
    free(interpreter->variables.variables);
    for (int i = 0; i < interpreter->shared_variables.count; i++) {
        free(interpreter->shared_variables.variables[i].name);
        if (interpreter->shared_variables.variables[i].type == VAR_STRING)
            free(interpreter->shared_variables.variables[i].value.string);
    }
    free(interpreter->shared_variables.variables);
    Category* current_cat = interpreter->categories;
    while (current_cat != NULL) {
        Category* next_cat = current_cat->next;
        free(current_cat->name);
        free(current_cat->statements);
        free(current_cat);
        current_cat = next_cat;
    }
    free(interpreter);
}

EvalResult evaluate_expression(Interpreter* interpreter, ASTNode* node) {
    if (!node) return create_number_result(0);
    switch (node->type) {
        case AST_NUMBER: return create_number_result(node->data.number.value);
        case AST_STRING: return create_string_result(node->data.string.value);
        case AST_IDENTIFIER: {
            Variable* var = get_variable(interpreter, node->data.identifier.name);
            if (var) {
                if (var->type == VAR_NUMBER)
                    return create_number_result(var->value.number);
                else if (var->type == VAR_STRING)
                    return create_string_result(var->value.string);
            }
            fprintf(stderr, "Runtime error: Undefined variable '%s'\n", node->data.identifier.name);
            return create_number_result(0);
        }
        case AST_BINARY_OP: {
            EvalResult left = evaluate_expression(interpreter, node->data.binary_op.left);
            EvalResult right = evaluate_expression(interpreter, node->data.binary_op.right);

            // 数値同士
            if (left.type == RESULT_NUMBER && right.type == RESULT_NUMBER) {
                double l = left.value.number, r = right.value.number, res = 0; int comparison = 0;
                switch (node->data.binary_op.operator) {
                    case TOKEN_PLUS: res = l + r; break;
                    case TOKEN_MINUS: res = l - r; break;
                    case TOKEN_MULTIPLY: res = l * r; break;
                    case TOKEN_DIVIDE: if (r != 0) res = l / r; else { fprintf(stderr, "Runtime error: Division by zero\n"); res = 0;} break;
                    case TOKEN_AT: res = l * r; break;
                    case TOKEN_YEN:
                    case TOKEN_BACKSLASH: if (r != 0) res = l / r; else { fprintf(stderr, "Runtime error: Division by zero\n"); res = 0;} break;
                    case TOKEN_MOD: res = fmod(l, r); break;
                    case TOKEN_GT: comparison = (l > r); break;
                    case TOKEN_LT: comparison = (l < r); break;
                    case TOKEN_GTE: comparison = (l >= r); break;
                    case TOKEN_LTE: comparison = (l <= r); break;
                    case TOKEN_EQ: comparison = (l == r); break;
                    case TOKEN_NEQ: comparison = (l != r); break;
                    case TOKEN_AMPERSAND: comparison = (l != 0 && r != 0); break;
                    case TOKEN_PIPE: comparison = (l != 0 || r != 0); break;
                    default: fprintf(stderr, "Runtime error: Unsupported binary operator on numbers\n"); break;
                }
                if (node->data.binary_op.operator >= TOKEN_GT && node->data.binary_op.operator <= TOKEN_PIPE)
                    return create_number_result(comparison ? 1.0 : 0.0);
                return create_number_result(res);
            }
            // 文字列同士
            else if (left.type == RESULT_STRING && right.type == RESULT_STRING) {
                int cmp = strcmp(left.value.string, right.value.string);
                int result = 0;
                switch (node->data.binary_op.operator) {
                    case TOKEN_EQ:  result = (cmp == 0); break;
                    case TOKEN_NEQ: result = (cmp != 0); break;
                    case TOKEN_GT:  result = (cmp > 0); break;
                    case TOKEN_LT:  result = (cmp < 0); break;
                    case TOKEN_GTE: result = (cmp >= 0); break;
                    case TOKEN_LTE: result = (cmp <= 0); break;
                    case TOKEN_PLUS: {
                        // 文字列連結
                        size_t len = strlen(left.value.string) + strlen(right.value.string) + 1;
                        char* result_str = malloc(len);
                        strcpy(result_str, left.value.string);
                        strcat(result_str, right.value.string);
                        free(left.value.string);
                        free(right.value.string);
                        EvalResult result_ret = create_string_result(result_str);
                        free(result_str);
                        return result_ret;
                    }
                    default:
                        fprintf(stderr, "Runtime error: Unsupported binary operator on strings\n");
                        free(left.value.string);
                        free(right.value.string);
                        return create_number_result(0);
                }
                free(left.value.string);
                free(right.value.string);
                return create_number_result(result ? 1.0 : 0.0);
            }
            // 片方が文字列
            else if (left.type == RESULT_STRING || right.type == RESULT_STRING) {
                if (node->data.binary_op.operator == TOKEN_PLUS) {
                    char l_str_buf[100], r_str_buf[100], *l_str, *r_str;
                    if (left.type == RESULT_NUMBER) { sprintf(l_str_buf, "%g", left.value.number); l_str = l_str_buf;} else { l_str = left.value.string;}
                    if (right.type == RESULT_NUMBER) { sprintf(r_str_buf, "%g", right.value.number); r_str = r_str_buf;} else { r_str = right.value.string;}
                    size_t len = strlen(l_str) + strlen(r_str) + 1;
                    char* result_str = malloc(len);
                    strcpy(result_str, l_str); strcat(result_str, r_str);
                    if (left.type == RESULT_STRING) free(left.value.string);
                    if (right.type == RESULT_STRING) free(right.value.string);
                    EvalResult result = create_string_result(result_str);
                    free(result_str);
                    return result;
                } else {
                    fprintf(stderr, "Runtime error: Unsupported binary operator on strings\n");
                    if (left.type == RESULT_STRING) free(left.value.string);
                    if (right.type == RESULT_STRING) free(right.value.string);
                    return create_number_result(0);
                }
            }
            return create_number_result(0);
        }
        case AST_UNARY_OP: {
            EvalResult operand = evaluate_expression(interpreter, node->data.unary_op.operand);
            if (operand.type == RESULT_NUMBER) {
                double val = operand.value.number, res = 0;
                switch (node->data.unary_op.operator) {
                    case TOKEN_PLUS: res = val; break;
                    case TOKEN_MINUS: res = -val; break;
                    case TOKEN_TILDE: res = (val == 0.0) ? 1.0 : 0.0; break;
                    default: fprintf(stderr, "Runtime error: Unsupported unary operator on number\n"); break;
                }
                return create_number_result(res);
            } else if (operand.type == RESULT_STRING) {
                if (node->data.unary_op.operator == TOKEN_TILDE) {
                    double res = (strlen(operand.value.string) == 0) ? 1.0 : 0.0;
                    free(operand.value.string);
                    return create_number_result(res);
                }
                fprintf(stderr, "Runtime error: Unsupported unary operator on string\n");
                free(operand.value.string);
                return create_number_result(0);
            }
        }
        default:
            fprintf(stderr, "Runtime error: Cannot evaluate AST type %d as expression\n", node->type);
            return create_number_result(0);
    }
}

void define_category(Interpreter* interpreter, const char* name, ASTNode** statements, int count) {
    Category* new_category = malloc(sizeof(Category));
    new_category->name = strdup(name);
    new_category->statements = statements;
    new_category->statement_count = count;
    new_category->next = interpreter->categories;
    interpreter->categories = new_category;
}

Category* find_category(Interpreter* interpreter, const char* name) {
    Category* current = interpreter->categories;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) return current;
        current = current->next;
    }
    return NULL;
}

void run_category(Interpreter* interpreter, const char* name) {
    Category* category = find_category(interpreter, name);
    if (!category) {
        fprintf(stderr, "Runtime error: Undefined category '%s'\n", name);
        return;
    }
    for (int i = 0; i < category->statement_count; i++)
        interpret(interpreter, category->statements[i]);
}

void execute_external_code(Interpreter* interpreter, const char* language, const char* code) {
    printf("External call: Language: %s, Code: %s\n", language, code);
    if (strcmp(language, "py") == 0) {
        printf("Executing Python code is not implemented in this C interpreter.\n");
    } else {
        fprintf(stderr, "Runtime error: Unsupported external language '%s'\n", language);
    }
}

void interpret(Interpreter* interpreter, ASTNode* ast) {
    if (!ast) return;
    switch (ast->type) {
        case AST_COMPOUND_STATEMENT:
            for (int i = 0; i < ast->data.compound_statement.statement_count; i++)
                interpret(interpreter, ast->data.compound_statement.statements[i]);
            break;
        case AST_ASSIGNMENT: {
            EvalResult result = evaluate_expression(interpreter, ast->data.assignment.expression);
            int is_shared = 0;
            set_variable(interpreter, ast->data.assignment.variable, result, is_shared);
            if (result.type == RESULT_STRING) free(result.value.string);
            break;
        }
        case AST_RE_ASSIGNMENT: {
            EvalResult result = evaluate_expression(interpreter, ast->data.assignment.expression);
            Variable* var = get_variable(interpreter, ast->data.assignment.variable);
            if (var) set_variable(interpreter, ast->data.assignment.variable, result, var->is_shared);
            else fprintf(stderr, "Runtime error: Attempted 're' assignment to undeclared variable '%s'\n", ast->data.assignment.variable);
            if (result.type == RESULT_STRING) free(result.value.string);
            break;
        }
        case AST_SUNUM_STATEMENT:
            set_shared_variable(interpreter, ast->data.assignment.variable); break;
        case AST_WRITE_STATEMENT: {
            EvalResult result = evaluate_expression(interpreter, ast->data.write_statement.expression);
            if (result.type == RESULT_STRING) { printf("%s\n", result.value.string); free(result.value.string);}
            else if (result.type == RESULT_NUMBER) printf("%g\n", result.value.number);
            break;
        }
        case AST_NUM_WRITE_STATEMENT: {
            Variable* var = get_variable(interpreter, ast->data.num_write_statement.variable_name);
            if (var) {
                if (var->type == VAR_NUMBER) printf("%g\n", var->value.number);
                else if (var->type == VAR_STRING) printf("%s\n", var->value.string);
            } else {
                fprintf(stderr, "Runtime error: Undefined variable '%s' for 'num write'\n", ast->data.num_write_statement.variable_name);
            }
            break;
        }
        case AST_IF_STATEMENT: {
            EvalResult condition_result = evaluate_expression(interpreter, ast->data.if_statement.condition);
            int is_true = 0;
            if (condition_result.type == RESULT_NUMBER) is_true = (condition_result.value.number != 0);
            else if (condition_result.type == RESULT_STRING) { is_true = (strlen(condition_result.value.string) > 0); free(condition_result.value.string);}
            if (is_true) interpret(interpreter, ast->data.if_statement.then_stmt);
            else if (ast->data.if_statement.else_stmt != NULL)
                interpret(interpreter, ast->data.if_statement.else_stmt);
            break;
        }
        case AST_CATEGORY_DEFINITION:
             define_category(interpreter, ast->data.category_definition.name, ast->data.category_definition.statements, ast->data.category_definition.statement_count);
             ast->data.category_definition.statements = NULL; 
             break;
        case AST_RUN_STATEMENT:
            run_category(interpreter, ast->data.run_statement.category_name); break;
        case AST_CALL_STATEMENT:
            execute_external_code(interpreter, ast->data.call_statement.language, ast->data.call_statement.code); break;
        default:
            if (ast->type >= AST_NUMBER && ast->type <= AST_UNARY_OP) {
                EvalResult result = evaluate_expression(interpreter, ast);
                if (result.type == RESULT_STRING) free(result.value.string);
            } else {
                fprintf(stderr, "Runtime error: Cannot interpret AST type %d\n", ast->type);
            }
            break;
    }
}