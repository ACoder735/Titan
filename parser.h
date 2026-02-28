#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// AST Node Types
typedef enum {
    AST_NUMBER, AST_STRING, AST_BOOL, AST_NULL, AST_IDENTIFIER,
    AST_ARRAY, // New
    AST_VARDECL, AST_ASSIGN, AST_BINOP, AST_UNARYOP,
    AST_CALL, AST_IF, AST_WHILE, AST_FOR, AST_FUNCDECL, AST_RETURN, AST_BLOCK
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char name[100];    // Variable names, Operators, Function names
    char value[100];   // Literal values
    
    struct ASTNode* left;   // Used for: Function args, Array items, Condition
    struct ASTNode* right;  // Used for: Value, Then-branch, Loop body
    struct ASTNode* next;   // Used for: Next item in list/block
    struct ASTNode* body;   // Used for: Else-branch, For-loop body
    struct ASTNode* step;   // Used for: For-loop step
} ASTNode;

typedef struct {
    Lexer* lexer;
    Token currentToken;
    Token nextToken; // Lookahead buffer
} Parser;

void initParser(Parser* parser, Lexer* lexer);
ASTNode* parseProgram(Parser* parser);

#endif