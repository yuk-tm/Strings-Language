#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

void print_help() {
    printf("Custom Language Interpreter\n");
    printf("Usage:\n");
    printf("  ./interpreter <filename>  - Execute a script file (Use interpreter instead of strings.exe)\n");
    printf("  ./interpreter -i          - Start interactive mode (REPL)\n");
    printf("  ./interpreter -h          - Show this help message\n\n");
    printf("Language Syntax Example:\n");
    printf("  # This is a comment\n");
    printf("  my_var = '10' / \n");
    printf("  num write my_var / \n");
    printf("  write \"Hello World!\" / \n");
    printf("  x = '5' +* '2' / # x is 10\n");
    printf("  x > '5' ? write \"YES\" ; ! write \"NO\" / \n");
}

void interactive_mode() {
    char input[2048];
    printf("Interactive Mode. Type 'exit/' to quit.\n");
    Interpreter* interpreter = interpreter_create();
    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "exit/") == 0) break;
        if (strlen(input) == 0) continue;
        TokenList tokens = tokenize(input);
        Parser* parser = parser_create(tokens);
        ASTNode* ast = parse(parser);
        if (ast) {
            interpret(interpreter, ast);
            ast_free(ast);
        }
        parser_free(parser);
        free_tokens(&tokens);
    }
    interpreter_free(interpreter);
    printf("Leaving interactive mode.\n");
}

void run_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) { perror("Error opening file"); return; }
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* content = malloc(fsize + 1);
    fread(content, 1, fsize, file);
    fclose(file);
    content[fsize] = '\0';
    TokenList tokens = tokenize(content);
    Parser* parser = parser_create(tokens);
    ASTNode* ast = parse(parser);
    if (ast) {
        Interpreter* interpreter = interpreter_create();
        interpret(interpreter, ast);
        interpreter_free(interpreter);
        ast_free(ast);
    } else {
        printf("Failed to parse the file.\n");
    }
    parser_free(parser);
    free_tokens(&tokens);
    free(content);
}

int main(int argc, char* argv[]) {
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "-h") == 0)) {
        print_help();
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "-i") == 0) {
        interactive_mode();
        return 0;
    }
    if (argc == 2) {
        run_file(argv[1]);
        return 0;
    }
    fprintf(stderr, "Invalid arguments. Use -h for help.\n");
    return 1;
}