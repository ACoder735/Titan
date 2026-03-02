#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "lexer.h"
#include "codegen.h"

int file_exists(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

char* readFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    return buffer;
}

void create_directory(const char *path) {
    char cmd[300];
    sprintf(cmd, "if not exist \"%s\" mkdir \"%s\"", path, path);
    system(cmd);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Titan Compiler v0.9.0\n");
        printf("Usage: titan090 <script.tn> [--no-build]\n");
        return 1;
    }

    char inputArg[256];
    strcpy(inputArg, argv[1]);

    int noBuild = 0;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--no-build") == 0) noBuild = 1;
    }

    // --- PATH LOGIC ---
    char projDir[256] = "TitanProjects";
    char inputFile[512];    // The actual source file path
    char scriptName[256];   // Just the filename (e.g., test.tn)
    char buildDir[512];     // The folder for generated files
    char cFile[512];
    char exeFile[512];

    // 1. Determine Input File Path
    // If input is just "test.tn", look in current directory first.
    if (strchr(inputArg, '\\') == NULL && strchr(inputArg, '/') == NULL) {
        // No path provided, assume current directory
        strcpy(inputFile, inputArg);
    } else {
        // Path provided, use it
        strcpy(inputFile, inputArg);
    }

    // 2. Check if file exists
    if (!file_exists(inputFile)) {
        printf("Error: Source file '%s' not found.\n", inputFile);
        return 1;
    }

    // 3. Extract Script Name
    char* lastSlash = strrchr(inputFile, '\\');
    if (!lastSlash) lastSlash = strrchr(inputFile, '/');
    if (lastSlash) strcpy(scriptName, lastSlash + 1);
    else strcpy(scriptName, inputFile);

    // 4. Determine Output Directory
    // We want: TitanProjects/<scriptname_no_ext>/
    // e.g., TitanProjects/test/
    char folderName[100];
    strcpy(folderName, scriptName);
    char* dot = strrchr(folderName, '.');
    if (dot) *dot = '\0'; // Remove extension

    sprintf(buildDir, "%s\\%s", projDir, folderName);
    
    // Create folders if they don't exist
    create_directory(projDir);
    create_directory(buildDir);

    // 5. Set Output File Paths
    // The generated files will be: TitanProjects/test/test.tn.c
    sprintf(cFile, "%s\\%s.c", buildDir, scriptName);
    sprintf(exeFile, "%s\\%s.exe", buildDir, scriptName);

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
            return 1; 
        }

        Lexer lexer;
        Parser parser;

        initLexer(&lexer, src);
        initParser(&parser, &lexer);

        ASTNode* root = parseProgram(&parser);
        
        if (!root) {
            printf("Compilation failed.\n");
            free(src);
            return 1;
        }

        // Generate code into the Build Dir
        generateCode(root, cFile);

        char cmd[1024];
        // Include the current directory for titan_runtime.h
        // Wait, runtime is generated in buildDir? No, codegen generates it where the C file is.
        // Let's verify codegen.c logic: it writes header next to the C file.
        // So we compile from buildDir.
        
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