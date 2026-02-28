#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Symbol Table ---
typedef struct { char name[100]; int scope; } Symbol;
#define MAX_SYMBOLS 1000
Symbol symbols[MAX_SYMBOLS];
int symbol_count = 0;
int current_scope = 0;

void resetSymbols() { symbol_count = 0; current_scope = 0; }
void enterScope() { current_scope++; }
void leaveScope() {
    int new_count = 0;
    for (int i = 0; i < symbol_count; i++) {
        if (symbols[i].scope < current_scope) symbols[new_count++] = symbols[i];
    }
    symbol_count = new_count;
    current_scope--;
}
void addSymbol(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0 && symbols[i].scope == current_scope) return;
    }
    strcpy(symbols[symbol_count].name, name);
    symbols[symbol_count].scope = current_scope;
    symbol_count++;
}
int hasSymbol(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) return 1;
    }
    return 0;
}

// --- Code Generation ---
void generateNode(ASTNode* node, FILE* out);
void generateBlock(ASTNode* block, FILE* out);

int getStepDirection(ASTNode* stepNode) {
    if (!stepNode || stepNode->type != AST_ASSIGN) return 0;
    ASTNode* expr = stepNode->right;
    if (expr && expr->type == AST_BINOP) {
        if (strcmp(expr->name, "+") == 0) return 1;
        if (strcmp(expr->name, "-") == 0) return -1;
    }
    return 0;
}

void generateNode(ASTNode* node, FILE* out) {
    if (!node) return;
    
    switch(node->type) {
        case AST_NUMBER: fprintf(out, "titan_num(%s)", node->value); break;
        case AST_STRING: fprintf(out, "titan_str(\"%s\")", node->value); break;
        case AST_BOOL: fprintf(out, "titan_num(%s)", node->value); break;
        case AST_NULL: fprintf(out, "TITAN_NULL"); break;
        case AST_IDENTIFIER: fprintf(out, "%s", node->name); break;
        
        case AST_ARRAY: {
            fprintf(out, "titan_new_array()");
            ASTNode* elem = node->left;
            while(elem) {
                fprintf(out, ", titan_arr_push_result("); 
                generateNode(elem, out); 
                fprintf(out, ")");
                elem = elem->next;
            }
            break;
        }

        case AST_VARDECL: {
            if (hasSymbol(node->name)) {
                fprintf(out, "%s = ", node->name);
                generateNode(node->right, out);
                fprintf(out, ";\n");
            } else {
                addSymbol(node->name);
                fprintf(out, "TitanVar %s = ", node->name);
                generateNode(node->right, out);
                fprintf(out, ";\n");
            }
            break;
        }
            
        case AST_ASSIGN:
            if (!hasSymbol(node->name)) {
                printf("Warning: Variable '%s' might not be declared.\n", node->name);
            }
            fprintf(out, "%s = ", node->name);
            generateNode(node->right, out);
            fprintf(out, ";\n");
            break;
            
        case AST_BINOP: {
            if (strcmp(node->name, "+") == 0) { fprintf(out, "titan_add("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "-") == 0) { fprintf(out, "titan_sub("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "*") == 0) { fprintf(out, "titan_mul("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "/") == 0) { fprintf(out, "titan_div("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "%") == 0) { fprintf(out, "titan_mod("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "==") == 0) { fprintf(out, "titan_eq("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "!=") == 0) { fprintf(out, "titan_neq("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "<") == 0) { fprintf(out, "titan_lt("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, ">") == 0) { fprintf(out, "titan_gt("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "<=") == 0) { fprintf(out, "titan_lte("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, ">=") == 0) { fprintf(out, "titan_gte("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "&&") == 0) { fprintf(out, "titan_and("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "||") == 0) { fprintf(out, "titan_or("); generateNode(node->left, out); fprintf(out, ", "); generateNode(node->right, out); fprintf(out, ")"); }
            break;
        }
        
        case AST_UNARYOP:
            if (strcmp(node->name, "!") == 0) { fprintf(out, "titan_not("); generateNode(node->left, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "-") == 0) { fprintf(out, "titan_neg("); generateNode(node->left, out); fprintf(out, ")"); }
            break;
        
        case AST_CALL: {
            if (strcmp(node->name, "Str") == 0) { fprintf(out, "titan_tostring("); generateNode(node->left, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "Num") == 0) { fprintf(out, "titan_tonum("); generateNode(node->left, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "len") == 0) { fprintf(out, "titan_len("); generateNode(node->left, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "Array") == 0) { fprintf(out, "titan_new_array()"); }
            
            else if (strcmp(node->name, "console.typeln") == 0) { fprintf(out, "titan_println("); generateNode(node->left, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "console.type") == 0) { fprintf(out, "titan_print("); generateNode(node->left, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "console.askuser") == 0) { fprintf(out, "titan_input("); generateNode(node->left, out); fprintf(out, ")"); }
            else if (strcmp(node->name, "console.clear") == 0) {fprintf(out, "titan_clear()");}
            else if (strcmp(node->name, "console.setTitle") == 0) {fprintf(out, "titan_settitle(");generateNode(node->left, out);fprintf(out, ")");}
            else if (strcmp(node->name, "console.color") == 0) {fprintf(out, "titan_color(");generateNode(node->left, out);fprintf(out, ")");}
            
            else if (strchr(node->name, '.') != NULL) {
                char objName[100]; 
                char methodName[100];
                strcpy(objName, node->name);
                char* dot = strchr(objName, '.');
                *dot = '\0'; 
                strcpy(methodName, dot + 1);

                if (strcmp(methodName, "push") == 0) {
                    fprintf(out, "titan_arr_push(%s, ", objName); 
                    generateNode(node->left, out); 
                    fprintf(out, ")");
                }
                else if (strcmp(methodName, "get") == 0) {
                    fprintf(out, "titan_arr_get(%s, (int)(", objName);
                    generateNode(node->left, out);
                    fprintf(out, ").num)");
                }
                else if (strcmp(methodName, "remove") == 0) {
                    fprintf(out, "titan_arr_remove(%s, (int)(", objName);
                    generateNode(node->left, out);
                    fprintf(out, ").num)");
                }
                else if (strcmp(methodName, "insert") == 0) {
                    ASTNode* idxArg = node->left;
                    ASTNode* valArg = idxArg->next;
                    fprintf(out, "titan_arr_insert(%s, (int)(", objName);
                    generateNode(idxArg, out);
                    fprintf(out, ").num, ");
                    generateNode(valArg, out);
                    fprintf(out, ")");
                }
                else if (strcmp(methodName, "replace") == 0) {
                    ASTNode* idxArg = node->left;
                    ASTNode* valArg = idxArg->next;
                    fprintf(out, "titan_arr_replace(%s, (int)(", objName);
                    generateNode(idxArg, out);
                    fprintf(out, ").num, ");
                    generateNode(valArg, out);
                    fprintf(out, ")");
                }
                else if (strcmp(methodName, "concat") == 0) {
                    fprintf(out, "titan_arr_concat(%s, ", objName);
                    generateNode(node->left, out);
                    fprintf(out, ")");
                }
            }
            else {
                fprintf(out, "f_%s(", node->name);
                ASTNode* arg = node->left;
                while(arg) { generateNode(arg, out); arg = arg->next; if (arg) fprintf(out, ", "); }
                fprintf(out, ")");
            }
            break;
        }
            
        case AST_IF:
            fprintf(out, "if (titan_bool("); generateNode(node->left, out); fprintf(out, ")) {\n");
            enterScope(); generateBlock(node->right, out); leaveScope(); 
            fprintf(out, "}\n");
            if (node->body) { 
                fprintf(out, "else {\n");
                enterScope(); generateBlock(node->body, out); leaveScope();
                fprintf(out, "}\n");
            }
            break;
            
        case AST_WHILE:
            fprintf(out, "while (titan_bool("); generateNode(node->left, out); fprintf(out, ")) {\n");
            enterScope(); generateBlock(node->right, out); leaveScope();
            fprintf(out, "}\n");
            break;
            
        case AST_FOR: {
            addSymbol(node->name);
            fprintf(out, "for (TitanVar %s = ", node->name);
            generateNode(node->left, out);
            fprintf(out, "; ");
            
            int dir = getStepDirection(node->step);
            if (dir >= 0) {
                fprintf(out, "titan_bool(titan_lt(%s, ", node->name);
                generateNode(node->right, out);
                fprintf(out, "))");
            } else {
                fprintf(out, "titan_bool(titan_gt(%s, ", node->name);
                generateNode(node->right, out);
                fprintf(out, "))");
            }
            fprintf(out, "; ");
            
            if (node->step && node->step->type == AST_ASSIGN) {
                fprintf(out, "%s = ", node->step->name);
                generateNode(node->step->right, out);
            }
            
            fprintf(out, ") {\n");
            enterScope(); generateBlock(node->body, out); leaveScope(); 
            fprintf(out, "}\n");
            break;
        }
            
        case AST_FUNCDECL:
            fprintf(out, "TitanVar f_%s(", node->name);
            ASTNode* arg = node->left;
            enterScope();
            while(arg) {
                fprintf(out, "TitanVar %s", arg->name); 
                addSymbol(arg->name);
                arg = arg->next;
                if (arg) fprintf(out, ", ");
            }
            fprintf(out, ") {\n");
            generateBlock(node->right, out);
            fprintf(out, "    return TITAN_NULL;\n");
            fprintf(out, "}\n\n");
            leaveScope();
            break;
            
        case AST_RETURN:
            fprintf(out, "return ");
            if(node->left) generateNode(node->left, out);
            else fprintf(out, "TITAN_NULL");
            fprintf(out, ";\n");
            break;

        default: break;
    }
}

void generateBlock(ASTNode* block, FILE* out) {
    if (!block) return;
    if (block->type == AST_BLOCK) {
        ASTNode* stmt = block->left;
        while (stmt) {
            generateNode(stmt, out);
            if (stmt->type == AST_CALL) fprintf(out, ";\n");
            stmt = stmt->next;
        }
    } else {
        generateNode(block, out);
        if (block->type == AST_CALL) fprintf(out, ";\n");
    }
}

void writeRuntimeHeader(const char* filename) {
    FILE* out = fopen(filename, "w"); if(!out) return;
    fprintf(out, "#ifndef TITAN_RUNTIME_H\n#define TITAN_RUNTIME_H\n\n");
    fprintf(out, "#include <windows.h>\n\n");
    fprintf(out, "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <math.h>\n\n");

    // Forward Declaration
    fprintf(out, "typedef struct TitanVar TitanVar;\n\n");

    // Array Struct
    fprintf(out, "typedef struct {\n");
    fprintf(out, "    TitanVar* data;\n");
    fprintf(out, "    int count;\n");
    fprintf(out, "    int capacity;\n");
    fprintf(out, "} TitanArray;\n\n");

    // TitanVar Struct
    fprintf(out, "struct TitanVar {\n");
    fprintf(out, "    int type; double num; char* str; TitanArray* arr;\n");
    fprintf(out, "};\n\n");

    fprintf(out, "#define TITAN_NULL ((TitanVar){0, 0, 0, 0})\n");
    fprintf(out, "#define titan_num(n) ((TitanVar){1, n, 0, 0})\n");
    fprintf(out, "#define titan_str(s) ((TitanVar){2, 0, s, 0})\n\n");
    
    // Boolean
    fprintf(out, "int titan_bool(TitanVar v) { if (v.type == 0) return 0; if (v.type == 1) return v.num != 0; if (v.type == 2) return v.str != NULL && strlen(v.str) > 0; return 0; }\n\n");
    
    // Array Functions
    fprintf(out, "TitanVar titan_new_array() {\n");
    fprintf(out, "    TitanArray* a = malloc(sizeof(TitanArray));\n");
    fprintf(out, "    a->data = malloc(sizeof(TitanVar) * 4);\n");
    fprintf(out, "    a->count = 0; a->capacity = 4;\n");
    fprintf(out, "    return (TitanVar){3, 0, 0, a};\n");
    fprintf(out, "}\n\n");

    fprintf(out, "void _titan_ensure_space(TitanArray* a, int n) {\n");
    fprintf(out, "    if (a->count + n >= a->capacity) {\n");
    fprintf(out, "        while (a->count + n >= a->capacity) a->capacity *= 2;\n");
    fprintf(out, "        a->data = realloc(a->data, sizeof(TitanVar) * a->capacity);\n");
    fprintf(out, "    }\n");
    fprintf(out, "}\n\n");

    fprintf(out, "TitanVar titan_arr_push_result(TitanVar list, TitanVar item) {\n");
    fprintf(out, "    if(list.type != 3) return TITAN_NULL;\n");
    fprintf(out, "    _titan_ensure_space(list.arr, 1);\n");
    fprintf(out, "    list.arr->data[list.arr->count++] = item;\n");
    fprintf(out, "    return list;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "void titan_arr_push(TitanVar list, TitanVar item) {\n"); 
    fprintf(out, "    if(list.type != 3) return;\n");
    fprintf(out, "    _titan_ensure_space(list.arr, 1);\n");
    fprintf(out, "    list.arr->data[list.arr->count++] = item;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "TitanVar titan_arr_get(TitanVar list, int index) {\n");
    fprintf(out, "    if(list.type != 3 || index < 0 || index >= list.arr->count) return TITAN_NULL;\n");
    fprintf(out, "    return list.arr->data[index];\n");
    fprintf(out, "}\n\n");

    fprintf(out, "void titan_arr_replace(TitanVar list, int index, TitanVar val) {\n");
    fprintf(out, "    if(list.type != 3 || index < 0 || index >= list.arr->count) return;\n");
    fprintf(out, "    list.arr->data[index] = val;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "void titan_arr_insert(TitanVar list, int index, TitanVar item) {\n");
    fprintf(out, "    if(list.type != 3 || index < 0 || index > list.arr->count) return;\n");
    fprintf(out, "    _titan_ensure_space(list.arr, 1);\n");
    fprintf(out, "    for (int i = list.arr->count; i > index; i--) list.arr->data[i] = list.arr->data[i-1];\n");
    fprintf(out, "    list.arr->data[index] = item;\n");
    fprintf(out, "    list.arr->count++;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "void titan_arr_remove(TitanVar list, int index) {\n");
    fprintf(out, "    if(list.type != 3 || index < 0 || index >= list.arr->count) return;\n");
    fprintf(out, "    for (int i = index; i < list.arr->count - 1; i++) list.arr->data[i] = list.arr->data[i+1];\n");
    fprintf(out, "    list.arr->count--;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "TitanVar titan_arr_concat(TitanVar a, TitanVar b) {\n");
    fprintf(out, "    if(a.type != 3 || b.type != 3) return TITAN_NULL;\n");
    fprintf(out, "    TitanVar result = titan_new_array();\n");
    fprintf(out, "    _titan_ensure_space(result.arr, a.arr->count + b.arr->count);\n");
    fprintf(out, "    for(int i=0; i<a.arr->count; i++) result.arr->data[result.arr->count++] = a.arr->data[i];\n");
    fprintf(out, "    for(int i=0; i<b.arr->count; i++) result.arr->data[result.arr->count++] = b.arr->data[i];\n");
    fprintf(out, "    return result;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "TitanVar titan_len(TitanVar v) {\n");
    fprintf(out, "    if (v.type == 3) return titan_num(v.arr->count);\n");
    fprintf(out, "    if (v.type == 2) return titan_num(strlen(v.str));\n");
    fprintf(out, "    return titan_num(0);\n");
    fprintf(out, "}\n\n");

    // Conversion
    fprintf(out, "char* _titan_val_to_str(TitanVar v) {\n");
    fprintf(out, "    char* buf = malloc(256); buf[0] = 0;\n");
    fprintf(out, "    if (v.type == 1) { sprintf(buf, \"%%g\", v.num); }\n");
    fprintf(out, "    else if (v.type == 2) { strcpy(buf, v.str); }\n");
    fprintf(out, "    else if (v.type == 0) { strcpy(buf, \"NULL\"); }\n");
    fprintf(out, "    else if (v.type == 3) {\n");
    fprintf(out, "        strcpy(buf, \"[\");\n");
    fprintf(out, "        for(int i=0; i<v.arr->count; i++) {\n");
    fprintf(out, "            if(i>0) strcat(buf, \", \");\n");
    fprintf(out, "            char* inner = _titan_val_to_str(v.arr->data[i]);\n");
    fprintf(out, "            strcat(buf, inner);\n");
    fprintf(out, "            free(inner);\n");
    fprintf(out, "        }\n");
    fprintf(out, "        strcat(buf, \"]\");\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return buf;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "TitanVar titan_tostring(TitanVar v) {\n");
    fprintf(out, "    return titan_str(_titan_val_to_str(v));\n");
    fprintf(out, "}\n\n");

    fprintf(out, "TitanVar titan_tonum(TitanVar v) { if (v.type == 1) return v; if (v.type == 2) return titan_num(atof(v.str)); return titan_num(0); }\n\n");
    
    // Print
    fprintf(out, "void titan_print(TitanVar v) {\n");
    fprintf(out, "    if (v.type == 2) printf(\"%%s\", v.str);\n"); 
    fprintf(out, "    else { printf(\"RUNTIME ERROR: Cannot print non-String types. Use Str(val).\\n\"); exit(1); }\n");
    fprintf(out, "}\n");
    fprintf(out, "void titan_println(TitanVar v) { titan_print(v); printf(\"\\n\"); }\n\n");
    
    // Input
    fprintf(out, "TitanVar titan_input(TitanVar prompt) { titan_print(prompt); fflush(stdout); char* buffer = malloc(256); if (fgets(buffer, 256, stdin) != NULL) { buffer[strcspn(buffer, \"\\n\")] = 0; buffer[strcspn(buffer, \"\\r\")] = 0; return titan_str(buffer); } return TITAN_NULL; }\n\n");
    
    // Math
    fprintf(out, "TitanVar titan_add(TitanVar a, TitanVar b) {\n");
    fprintf(out, "    if (a.type == 1 && b.type == 1) return titan_num(a.num + b.num);\n"); 
    fprintf(out, "    if (a.type == 2 && b.type == 2) { int len = strlen(a.str) + strlen(b.str) + 1; char* res = malloc(len); strcpy(res, a.str); strcat(res, b.str); return titan_str(res); }\n"); 
    fprintf(out, "    if (a.type == 3 && b.type == 3) return titan_arr_concat(a, b);\n");
    fprintf(out, "    printf(\"RUNTIME ERROR: Cannot add mismatched types.\\n\"); exit(1);\n"); 
    fprintf(out, "}\n\n");
    
    fprintf(out, "TitanVar titan_sub(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf(\"RUNTIME ERROR: '-' requires Numbers.\\n\"); exit(1); } return titan_num(a.num - b.num); }\n");
    fprintf(out, "TitanVar titan_mul(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf(\"RUNTIME ERROR: '*' requires Numbers.\\n\"); exit(1); } return titan_num(a.num * b.num); }\n");
    fprintf(out, "TitanVar titan_div(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf(\"RUNTIME ERROR: '/' requires Numbers.\\n\"); exit(1); } return titan_num(a.num / b.num); }\n");
    fprintf(out, "TitanVar titan_mod(TitanVar a, TitanVar b) { if (a.type != 1 || b.type != 1) { printf(\"RUNTIME ERROR: '%%%%' requires Numbers.\\n\"); exit(1); } return titan_num(fmod(a.num, b.num)); }\n\n");

    fprintf(out, "TitanVar titan_neg(TitanVar a) { if(a.type != 1) { printf(\"RUNTIME ERROR: '-' requires Number.\\n\"); exit(1); } return titan_num(-a.num); }\n");
    fprintf(out, "TitanVar titan_not(TitanVar a) { return titan_num(!titan_bool(a)); }\n");
    fprintf(out, "TitanVar titan_and(TitanVar a, TitanVar b) { return titan_num(titan_bool(a) && titan_bool(b)); }\n");
    fprintf(out, "TitanVar titan_or(TitanVar a, TitanVar b) { return titan_num(titan_bool(a) || titan_bool(b)); }\n\n");

    // Comparisons
    fprintf(out, "TitanVar titan_eq(TitanVar a, TitanVar b) { if (a.type != b.type) return titan_num(0); if (a.type == 0) return titan_num(1); if (a.type == 2) return titan_num(strcmp(a.str, b.str) == 0); return titan_num(a.num == b.num); }\n");
    fprintf(out, "TitanVar titan_neq(TitanVar a, TitanVar b) { return titan_num(!titan_bool(titan_eq(a, b))); }\n");
    fprintf(out, "TitanVar titan_lt(TitanVar a, TitanVar b) { return titan_num(a.num < b.num); }\n");
    fprintf(out, "TitanVar titan_gt(TitanVar a, TitanVar b) { return titan_num(a.num > b.num); }\n");
    fprintf(out, "TitanVar titan_lte(TitanVar a, TitanVar b) { return titan_num(a.num <= b.num); }\n");
    fprintf(out, "TitanVar titan_gte(TitanVar a, TitanVar b) { return titan_num(a.num >= b.num); }\n\n");

    // Console Helpers
    fprintf(out, "void titan_clear() { system(\"cls\"); }\n"); 
    fprintf(out, "void titan_settitle(TitanVar t) { if(t.type != 2) return; char cmd[256]; sprintf(cmd, \"title %%s\", t.str); system(cmd); }\n\n");
    fprintf(out, "void titan_color(TitanVar c) {\n");
    fprintf(out, "    if (c.type == 1) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (int)c.num);\n");
    fprintf(out, "}\n\n");
    
    fprintf(out, "#endif\n");
    fclose(out);
}

void generateCode(ASTNode* root, const char* filename) {
    char headerPath[256]; strcpy(headerPath, filename);
    char* lastSlash = strrchr(headerPath, '\\');
    if (lastSlash) { *(lastSlash + 1) = 0; strcat(headerPath, "titan_runtime.h"); }
    else strcpy(headerPath, "titan_runtime.h");
    writeRuntimeHeader(headerPath);

    FILE* out = fopen(filename, "w");
    fprintf(out, "// Generated by Titan Compiler v0.9.0 .\n");
    fprintf(out, "#include \"titan_runtime.h\"\n\n");

    ASTNode* current = root->left;
    while(current) { if (current->type == AST_FUNCDECL) { resetSymbols(); generateNode(current, out); } current = current->next; }

    fprintf(out, "// MAIN\nint main() {\n");
    resetSymbols(); enterScope();
    current = root->left;
    while(current) { if (current->type != AST_FUNCDECL) { generateNode(current, out); if (current->type == AST_CALL) fprintf(out, ";\n"); } current = current->next; }
    fprintf(out, "    system(\"pause\");\n    return 0;\n}\n");
    fclose(out);
}