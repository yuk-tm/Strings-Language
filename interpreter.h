#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

// 変数の値（仕様準拠）
typedef struct {
    char* name;
    union {
        double number;
        char* string;
        struct {
            struct Variable** items;  // リスト要素
            int count;
            int capacity;
        } list;
    } value;
    enum {
        VAR_NUMBER,
        VAR_STRING, 
        VAR_LIST
    } type;
    int is_shared;  // sunum変数かどうか
} Variable;

// 変数テーブル
typedef struct {
    Variable* variables;
    int count;
    int capacity;
} VariableTable;

// カテゴリ（関数・プロシージャ）
typedef struct Category {
    char* name;
    ASTNode** statements;
    int statement_count;
    struct Category* next;
} Category;

// インタープリター
typedef struct {
    VariableTable variables;
    VariableTable shared_variables;  // sunum変数用
    Category* categories;            // カテゴリリスト
} Interpreter;

// 評価結果
typedef struct {
    union {
        double number;
        char* string;
        Variable* list;
    } value;
    enum {
        RESULT_NUMBER,
        RESULT_STRING,
        RESULT_LIST
    } type;
} EvalResult;

// 関数宣言
Interpreter* interpreter_create();
void interpreter_free(Interpreter* interpreter);
void interpret(Interpreter* interpreter, ASTNode* ast);

// 変数操作
void set_variable(Interpreter* interpreter, const char* name, EvalResult result, int is_shared);
Variable* get_variable(Interpreter* interpreter, const char* name);
void set_shared_variable(Interpreter* interpreter, const char* name);

// 式評価
EvalResult evaluate_expression(Interpreter* interpreter, ASTNode* node);

// カテゴリ操作
void define_category(Interpreter* interpreter, const char* name, ASTNode** statements, int count);
void run_category(Interpreter* interpreter, const char* name);

// 外部言語連携
void execute_external_code(Interpreter* interpreter, const char* language, const char* code);

// ユーティリティ
void print_result(EvalResult result);
EvalResult create_number_result(double value);
EvalResult create_string_result(const char* value);
void free_result(EvalResult result);

#endif