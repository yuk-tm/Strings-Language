#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

typedef struct Variable {
    char* name;
    union {
        double number;
        char* string;
    } value;
    enum { VAR_NUMBER, VAR_STRING } type;
    int is_shared;
} Variable;

typedef struct {
    Variable* variables;
    int count;
    int capacity;
} VariableTable;

typedef struct Category {
    char* name;
    ASTNode** statements;
    int statement_count;
    struct Category* next;
} Category;

typedef struct {
    VariableTable variables;
    VariableTable shared_variables;
    Category* categories;
} Interpreter;

typedef struct {
    union {
        double number;
        char* string;
    } value;
    enum { RESULT_NUMBER, RESULT_STRING } type;
} EvalResult;

Interpreter* interpreter_create();
void interpreter_free(Interpreter* interpreter);
void interpret(Interpreter* interpreter, ASTNode* ast);

void set_variable(Interpreter* interpreter, const char* name, EvalResult result, int is_shared);
Variable* get_variable(Interpreter* interpreter, const char* name);
void set_shared_variable(Interpreter* interpreter, const char* name);

EvalResult evaluate_expression(Interpreter* interpreter, ASTNode* node);

void define_category(Interpreter* interpreter, const char* name, ASTNode** statements, int count);
void run_category(Interpreter* interpreter, const char* name);

void execute_external_code(Interpreter* interpreter, const char* language, const char* code);

#endif