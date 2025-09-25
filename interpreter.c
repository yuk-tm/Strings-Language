#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter.h"

// インタープリターを作成
Interpreter* interpreter_create() {
    Interpreter* interpreter = malloc(sizeof(Interpreter));
    
    // ローカル変数テーブル初期化
    interpreter->variables.variables = malloc(sizeof(Variable) * 10);
    interpreter->variables.count = 0;
    interpreter->variables.capacity = 10;
    
    // 共有変数テーブル初期化
    interpreter->shared_variables.variables = malloc(sizeof(Variable) * 10);
    interpreter->shared_variables.count = 0;
    interpreter->shared_variables.capacity = 10;
    
    // カテゴリリスト初期化
    interpreter->categories = NULL;
    
    return interpreter;
}

// インタープリターを解放
void interpreter_free(Interpreter* interpreter) {
    if (!interpreter) return;
    
    // ローカル変数を解放
    for (int i = 0; i < interpreter->variables.count; i++) {
        free(interpreter->variables.variables[i].name);
        if (interpreter->variables.variables[i].type == VAR_STRING) {
            free(interpreter->variables.variables[i].value.string);
        }
        // TODO: リスト解放
    }
    free(interpreter->variables.variables);
    
    // 共有変数を解放
    for (int i = 0; i < interpreter->shared_variables.count; i++) {
        free(interpreter->shared_variables.variables[i].name);
        if (interpreter->shared_variables.variables[i].type == VAR_STRING) {
            free(interpreter->shared_variables.variables[i].value.string);
        }
        // TODO: リスト解放
    }
    free(interpreter->shared_variables.variables);
    
    // カテゴリを解放
    Category* current = interpreter->categories;
    while (current) {
        Category* next = current->next;
        free(current->name);
        // TODO: statements解放
        free(current);
        current = next;
    }
    
    free(interpreter);
}

// 結果作成ユーティリティ
EvalResult create_number_result(double value) {
    EvalResult result;
    result.type = RESULT_NUMBER;
    result.value.number = value;
    return result;
}

EvalResult create_string_result(const char* value) {
    EvalResult result;
    result.type = RESULT_STRING;
    result.value.string = malloc(strlen(value) + 1);
    strcpy(result.value.string, value);
    return result;
}

void free_result(EvalResult result) {
    if (result.type == RESULT_STRING && result.value.string) {
        free(result.value.string);
    }
    // TODO: リスト解放
}

void print_result(EvalResult result) {
    switch (result.type) {
        case RESULT_NUMBER:
            if (result.value.number == (int)result.value.number) {
                printf("%.0f\n", result.value.number);
            } else {
                printf("%g\n", result.value.number);
            }
            break;
        case RESULT_STRING:
            printf("%s\n", result.value.string);
            break;
        case RESULT_LIST:
            printf("[List]\n");  // TODO: リスト表示実装
            break;
    }
}

// 変数を設定
void set_variable(Interpreter* interpreter, const char* name, EvalResult result, int is_shared) {
    VariableTable* table = is_shared ? &interpreter->shared_variables : &interpreter->variables;
    
    // 既存の変数をチェック
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->variables[i].name, name) == 0) {
            // 既存の値を解放
            if (table->variables[i].type == VAR_STRING) {
                free(table->variables[i].value.string);
            }
            
            // 新しい値を設定
            switch (result.type) {
                case RESULT_NUMBER:
                    table->variables[i].type = VAR_NUMBER;
                    table->variables[i].value.number = result.value.number;
                    break;
                case RESULT_STRING:
                    table->variables[i].type = VAR_STRING;
                    table->variables[i].value.string = malloc(strlen(result.value.string) + 1);
                    strcpy(table->variables[i].value.string, result.value.string);
                    break;
                case RESULT_LIST:
                    // TODO: リスト設定
                    break;
            }
            return;
        }
    }
    
    // 新しい変数を追加
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->variables = realloc(table->variables, sizeof(Variable) * table->capacity);
    }
    
    Variable* var = &table->variables[table->count];
    var->name = malloc(strlen(name) + 1);
    strcpy(var->name, name);
    var->is_shared = is_shared;
    
    switch (result.type) {
        case RESULT_NUMBER:
            var->type = VAR_NUMBER;
            var->value.number = result.value.number;
            break;
        case RESULT_STRING:
            var->type = VAR_STRING;
            var->value.string = malloc(strlen(result.value.string) + 1);
            strcpy(var->value.string, result.value.string);
            break;
        case RESULT_LIST:
            // TODO: リスト設定
            break;
    }
    
    table->count++;
}

// 変数を取得
Variable* get_variable(Interpreter* interpreter, const char* name) {
    // まず共有変数から探す
    for (int i = 0; i < interpreter->shared_variables.count; i++) {
        if (strcmp(interpreter->shared_variables.variables[i].name, name) == 0) {
            return &interpreter->shared_variables.variables[i];
        }
    }
    
    // 次にローカル変数から探す
    for (int i = 0; i < interpreter->variables.count; i++) {
        if (strcmp(interpreter->variables.variables[i].name, name) == 0) {
            return &interpreter->variables.variables[i];
        }
    }
    
    return NULL;
}

// 共有変数を設定
void set_shared_variable(Interpreter* interpreter, const char* name) {
    // 共有変数テーブルに空のエントリを作成
    EvalResult empty_result = create_string_result("");  // 空文字列で初期化
    set_variable(interpreter, name, empty_result, 1);
    free_result(empty_result);
}

// 式を評価
EvalResult evaluate_expression(Interpreter* interpreter, ASTNode* node) {
    EvalResult result = create_number_result(0);
    
    if (!node) {
        return result;
    }
    
    switch (node->type) {
        case AST_NUMBER:
            result = create_number_result(node->data.number.value);
            break;
            
        case AST_STRING:
            result = create_string_result(node->data.string.value);
            break;
            
        case AST_IDENTIFIER: {
            Variable* var = get_variable(interpreter, node->data.identifier.name);
            if (var) {
                switch (var->type) {
                    case VAR_NUMBER:
                        result = create_number_result(var->value.number);
                        break;
                    case VAR_STRING:
                        result = create_string_result(var->value.string);
                        break;
                    case VAR_LIST:
                        // TODO: リスト処理
                        result = create_string_result("[List]");
                        break;
                }
            } else {
                printf("Error: Undefined variable '%s'\n", node->data.identifier.name);
                result = create_number_result(0);
            }
            break;
        }
        
        case AST_BINARY_OP: {
            EvalResult left = evaluate_expression(interpreter, node->data.binary_op.left);
            EvalResult right = evaluate_expression(interpreter, node->data.binary_op.right);
            
            // 文字列連結
            if (node->data.binary_op.operator == TOKEN_PLUS && 
                (left.type == RESULT_STRING || right.type == RESULT_STRING)) {
                
                char left_str[100], right_str[100];
                if (left.type == RESULT_STRING) {
                    strcpy(left_str, left.value.string);
                } else {
                    sprintf(left_str, "%.0f", left.value.number);
                }
                
                if (right.type == RESULT_STRING) {
                    strcpy(right_str, right.value.string);
                } else {
                    sprintf(right_str, "%.0f", right.value.number);
                }
                
                char* combined = malloc(strlen(left_str) + strlen(right_str) + 1);
                strcpy(combined, left_str);
                strcat(combined, right_str);
                
                result = create_string_result(combined);
                free(combined);
            }
            // 数値演算
            else if (left.type == RESULT_NUMBER && right.type == RESULT_NUMBER) {
                switch (node->data.binary_op.operator) {
                    case TOKEN_PLUS:
                        result = create_number_result(left.value.number + right.value.number);
                        break;
                    case TOKEN_MINUS:
                        result = create_number_result(left.value.number - right.value.number);
                        break;
                    case TOKEN_AT:  // @ = 乗算
                        result = create_number_result(left.value.number * right.value.number);
                        break;
                    case TOKEN_BACKSLASH:  // \ = 除算
                    case TOKEN_YEN:        // ¥ = 除算
                        if (right.value.number != 0) {
                            result = create_number_result(left.value.number / right.value.number);
                        } else {
                            printf("Error: Division by zero\n");
                            result = create_number_result(0);
                        }
                        break;
                    case TOKEN_MOD:
                        if (right.value.number != 0) {
                            result = create_number_result(fmod(left.value.number, right.value.number));
                        } else {
                            printf("Error: Division by zero\n");
                            result = create_number_result(0);
                        }
                        break;
                    case TOKEN_GT:
                        result = create_number_result(left.value.number > right.value.number ? 1 : 0);
                        break;
                    case TOKEN_LT:
                        result = create_number_result(left.value.number < right.value.number ? 1 : 0);
                        break;
                    case TOKEN_GTE:
                        result = create_number_result(left.value.number >= right.value.number ? 1 : 0);
                        break;
                    case TOKEN_LTE:
                        result = create_number_result(left.value.number <= right.value.number ? 1 : 0);
                        break;
                    case TOKEN_EQ:
                        result = create_number_result(left.value.number == right.value.number ? 1 : 0);
                        break;
                    case TOKEN_NEQ:
                        result = create_number_result(left.value.number != right.value.number ? 1 : 0);
                        break;
                    case TOKEN_AMPERSAND:  // & = AND
                        result = create_number_result((left.value.number != 0 && right.value.number != 0) ? 1 : 0);
                        break;
                    case TOKEN_PIPE:  // | = OR
                        result = create_number_result((left.value.number != 0 || right.value.number != 0) ? 1 : 0);
                        break;
                    default:
                        printf("Error: Unsupported operator\n");
                        result = create_number_result(0);
                        break;
                }
            } else {
                printf("Error: Type mismatch in operation\n");
                result = create_number_result(0);
            }
            
            // 一時的な結果を解放
            free_result(left);
            free_result(right);
            break;
        }
        
        case AST_UNARY_OP: {
            EvalResult operand = evaluate_expression(interpreter, node->data.unary_op.operand);
            
            switch (node->data.unary_op.operator) {
                case TOKEN_TILDE:  // ~ = NOT
                    if (operand.type == RESULT_NUMBER) {
                        result = create_number_result(operand.value.number == 0 ? 1 : 0);
                    } else {
                        result = create_number_result(0);
                    }
                    break;
                default:
                    result = operand;
                    break;
            }
            
            if (node->data.unary_op.operator != TOKEN_TILDE) {
                free_result(operand);
            }
            break;
        }
        
        default:
            printf("Error: Cannot evaluate expression\n");
            result = create_number_result(0);
            break;
    }
    
    return result;
}

// ASTを実行
void interpret(Interpreter* interpreter, ASTNode* ast) {
    if (!ast) return;
    
    switch (ast->type) {
        case AST_ASSIGNMENT: {
            EvalResult result = evaluate_expression(interpreter, ast->data.assignment.value);
            set_variable(interpreter, ast->data.assignment.variable, result, 0);
            free_result(result);
            break;
        }
        
        case AST_RE_ASSIGNMENT: {
            EvalResult result = evaluate_expression(interpreter, ast->data.re_assignment.value);
            set_variable(interpreter, ast->data.re_assignment.variable, result, 0);
            free_result(result);
            break;
        }
        
        case AST_SUNUM_STATEMENT: {
            set_shared_variable(interpreter, ast->data.sunum_statement.variable);
            break;
        }
        
        case AST_WRITE_STATEMENT: {
            EvalResult result = evaluate_expression(interpreter, ast->data.write_statement.expression);
            print_result(result);
            free_result(result);
            break;
        }
        
        case AST_IF_STATEMENT: {
            EvalResult condition = evaluate_expression(interpreter, ast->data.if_statement.condition);
            int is_true = 0;
            
            switch (condition.type) {
                case RESULT_NUMBER:
                    is_true = condition.value.number != 0;
                    break;
                case RESULT_STRING:
                    is_true = strlen(condition.value.string) > 0;
                    break;
                case RESULT_LIST:
                    is_true = 1;  // リストは常にtrue
                    break;
            }
            
            if (is_true) {
                interpret(interpreter, ast->data.if_statement.then_stmt);
            } else if (ast->data.if_statement.else_stmt) {
                interpret(interpreter, ast->data.if_statement.else_stmt);
            }
            
            free_result(condition);
            break;
        }
        
        case AST_COMPOUND_STATEMENT: {
            // 複合文（;で区切られた複数文）を実行
            for (int i = 0; i < ast->data.compound_statement.count; i++) {
                interpret(interpreter, ast->data.compound_statement.statements[i]);
            }
            break;
        }
        
        case AST_RUN_STATEMENT: {
            run_category(interpreter, ast->data.run_statement.category_name);
            break;
        }
        
        case AST_CALL_STATEMENT: {
            execute_external_code(interpreter, ast->data.call_statement.language, ast->data.call_statement.code);
            break;
        }
        
        default:
            printf("Error: Cannot execute statement\n");
            break;
    }
}

// カテゴリ定義
void define_category(Interpreter* interpreter, const char* name, ASTNode** statements, int count) {
    Category* category = malloc(sizeof(Category));
    category->name = malloc(strlen(name) + 1);
    strcpy(category->name, name);
    
    category->statements = malloc(sizeof(ASTNode*) * count);
    for (int i = 0; i < count; i++) {
        category->statements[i] = statements[i];
    }
    category->statement_count = count;
    
    // リストに追加
    category->next = interpreter->categories;
    interpreter->categories = category;
}

// カテゴリ実行
void run_category(Interpreter* interpreter, const char* name) {
    Category* current = interpreter->categories;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            // カテゴリの全ての文を実行
            for (int i = 0; i < current->statement_count; i++) {
                interpret(interpreter, current->statements[i]);
            }
            return;
        }
        current = current->next;
    }
    
    printf("Error: Category '%s' not found\n", name);
}

// 外部言語実行（基本実装）
void execute_external_code(Interpreter* interpreter, const char* language, const char* code) {
    (void)interpreter;  // 未使用パラメータ警告を避ける
    
    if (strcmp(language, "py") == 0 || strcmp(language, "python") == 0) {
        printf("# Executing Python code:\n");
        printf("%s\n", code ? code : "(no code provided)");
        printf("# (Python execution not implemented yet)\n");
    } else {
        printf("Error: Unsupported language '%s'\n", language);
    }
}