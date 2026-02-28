#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>      // For detailed error messages
#include "parser.h"
#include "lexer.h"
#include "codegen.h"

// Helper to check if a file exists
int file_exists(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

// Helper to read file content
char* readFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = malloc(length + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    return buffer;
}

// Helper to create directory (Windows specific)
void create_directory(const char *path) {
    char cmd[300];
    // This Windows command creates the folder only if it doesn't exist
    sprintf(cmd, "if not exist \"%s\" mkdir \"%s\"", path, path);
    system(cmd);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Titan Compiler v0.9.0\n");
        printf("Usage: titan090 <script.tn> [--no-build]\n");
        return 1;
    }

    char* inputArg = argv[1];
    int noBuild = 0;
    
    // Check for flags
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--no-build") == 0) {
            noBuild = 1;
        }
    }

    // --- PATH LOGIC ---
    // Changed to PascalCase: TitanProjects
    char projDir[256] = "TitanProjects"; 
    char inputFile[512];
    char cFile[512];
    char exeFile[512];

    // 1. Ensure Project Directory exists
    create_directory(projDir);

    // 2. Determine input file path
    // If user provided a path (contains \ or /), use it as is.
    // Otherwise, assume it's in the Project Directory.
    if (strchr(inputArg, '\\') != NULL || strchr(inputArg, '/') != NULL) {
        strcpy(inputFile, inputArg);
    } else {
        // First check Project Folder
        sprintf(inputFile, "%s\\%s", projDir, inputArg);
        if (!file_exists(inputFile)) {
            // If not in project folder, check current directory
            if (file_exists(inputArg)) {
                strcpy(inputFile, inputArg);
            } else {
                // Neither found, stick with project path (for error message)
                sprintf(inputFile, "%s\\%s", projDir, inputArg);
            }
        }
    }

    // 3. Derive C and Exe paths
    strcpy(cFile, inputFile);
    strcat(cFile, ".c"); 
    
    strcpy(exeFile, inputFile);
    strcat(exeFile, ".exe"); 

    // --- RUN LOGIC ---

    if (noBuild) {
        if (!file_exists(exeFile)) {
            printf("Error: Executable '%s' not found.\n", exeFile);
            return 1;
        }
        printf("Running %s (No Build)...\n", exeFile);
        system(exeFile);
    } else {
        printf("Compiling: %s...\n", inputFile);
        
        char* src = readFile(inputFile);
        if (!src) { 
            printf("CRITICAL ERROR: Could not read source file.\n");
            printf("File Path: '%s'\n", inputFile);
            printf("System Error: %s\n", strerror(errno));
            printf("Suggestion: Create the file or check the spelling.\n");
            return 1; 
        }

        Lexer lexer;
        Parser parser;

        initLexer(&lexer, src);
        initParser(&parser, &lexer);

        ASTNode* root = parseProgram(&parser);
        
        if (!root) {
            printf("Compilation failed (Parser Error).\n");
            free(src);
            return 1;
        }

        generateCode(root, cFile);

        char cmd[1024];
        sprintf(cmd, "gcc \"%s\" -o \"%s\" -w", cFile, exeFile);
        
        int compileResult = system(cmd);
        
        if (compileResult != 0) {
            printf("GCC Compilation Error.\n");
            free(src);
            return 1;
        }

        printf("Build Successful. Running...\n\n");
        system(exeFile);
        
        free(src);
    }

    return 0;
}