#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

void print_help() {
    printf("Strings Programming Language v0.11 (Spec Compliant)\n");
    printf("Usage:\n");
    printf("  strings <filename.str>  - Execute file\n");
    printf("  strings -i              - Interactive mode\n");
    printf("  strings -h              - Show help\n");
    printf("\nLanguage Features:\n");
    printf("  Numbers:    '5, '10.5   (prefix with ')\n");
    printf("  Strings:    hello       (no quotes needed)\n");
    printf("  Math:       @ (mul), \\ or ¥ (div), %% (mod)\n");
    printf("  Logic:      & (and), | (or), ~ (not)\n");
    printf("  Comments:   # comment text\n");
}

// Interactive mode (REPL) - Spec compliant
void interactive_mode() {
    char input[1000];
    printf("Strings Language v0.11 Interactive Mode\n");
    printf("Type 'exit' to quit, 'help' for syntax help\n");
    printf("Examples:\n");
    printf("  write hello /               # Output: hello\n");
    printf("  name = world /              # Assign string\n");
    printf("  age = '25 /                 # Assign number\n");
    printf("  '5 > '3 ? write bigger /    # Condition\n\n");
    
    // Create interpreter
    Interpreter* interpreter = interpreter_create();
    
    while (1) {
        printf("strings> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        // Exit command
        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        
        // Skip empty lines
        if (strlen(input) == 0) {
            continue;
        }
        
        // Help display
        if (strcmp(input, "help") == 0) {
            printf("Strings Language v0.11 Syntax:\n");
            printf("Variables:\n");
            printf("  name = value /          # Assign\n");
            printf("  re name newvalue /      # Reassign\n");
            printf("  sunum varname /         # Shared variable\n");
            printf("Numbers:\n");
            printf("  '5, '10.5               # Prefix with '\n");
            printf("Strings:\n");
            printf("  hello, world            # No quotes needed\n");
            printf("Math:\n");
            printf("  '5 + '3 /               # Addition: 8\n");
            printf("  '10 @ '2 /              # Multiplication: 20\n");
            printf("  '10 \\\\ '2 /              # Division: 5\n");
            printf("Conditions:\n");
            printf("  '5 > '3 ? write yes / ! write no /\n");
            printf("Commands:\n");
            printf("  write value /           # Output\n");
            printf("  run category /          # Execute category\n");
            printf("  call py / code /        # External language\n");
            continue;
        }
        
        // Lexical analysis
        TokenList tokens = tokenize(input);
        if (tokens.count == 0 || (tokens.count > 0 && tokens.tokens[0].type == TOKEN_ERROR)) {
            printf("Error: Could not parse input\n");
            free_tokens(&tokens);
            continue;
        }
        
        // Parse
        Parser* parser = parser_create(tokens);
        ASTNode* ast = parse(parser);
        
        if (!ast) {
            printf("Error: Parse failed\n");
            parser_free(parser);
            free_tokens(&tokens);
            continue;
        }
        
        // Execute
        interpret(interpreter, ast);
        
        // Cleanup
        ast_free(ast);
        parser_free(parser);
        free_tokens(&tokens);
    }
    
    // Free interpreter
    interpreter_free(interpreter);
}

// File execution mode - Spec compliant
void run_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file '%s'\n", filename);
        return;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read file content
    char* content = malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    
    printf("Executing Strings v0.11 file '%s'...\n", filename);
    
    // Create interpreter
    Interpreter* interpreter = interpreter_create();
    
    // Process line by line
    char* line = strtok(content, "\n");
    while (line != NULL) {
        // Skip comment lines (# ...)
        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '#' || strlen(trimmed) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        // Lex, parse, execute
        TokenList tokens = tokenize(line);
        if (tokens.count > 1) {  // Has tokens other than EOF
            Parser* parser = parser_create(tokens);
            ASTNode* ast = parse(parser);
            
            if (ast) {
                interpret(interpreter, ast);
                ast_free(ast);
            }
            
            parser_free(parser);
        }
        free_tokens(&tokens);
        
        line = strtok(NULL, "\n");
    }
    
    // Cleanup
    interpreter_free(interpreter);
    free(content);
    printf("Execution completed\n");
}

int main(int argc, char* argv[]) {
    printf("=== Strings Programming Language v0.11 ===\n");
    printf("Specification Compliant Version\n\n");
    
    // Process arguments
    if (argc == 1) {
        // No arguments: show help
        print_help();
        return 0;
    }
    
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_help();
            return 0;
        }
        
        if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0) {
            interactive_mode();
            return 0;
        }
        
        // Treat as filename
        run_file(argv[1]);
        return 0;
    }
    
    printf("Error: Too many arguments\n");
    print_help();
    return 1;
}