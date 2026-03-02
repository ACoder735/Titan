#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// AST Node Types
typedef enum {
    AST_NUMBER, AST_STRING, AST_BOOL, AST_NULL, AST_IDENTIFIER,
    AST_ARRAY, 
    AST_VARDECL, AST_ASSIGN, AST_BINOP, AST_UNARYOP,
    AST_CALL, AST_IF, AST_WHILE, AST_FOR, AST_FUNCDECL, AST_RETURN, AST_BLOCK
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char name[100];    
    char value[100];   
    
    struct ASTNode* left;   
    struct ASTNode* right;  
    struct ASTNode* next;   
    struct ASTNode* body;   
    struct ASTNode* step;   
} ASTNode;

typedef struct {
    Lexer* lexer;
    Token currentToken;
    Token nextToken; 
} Parser;

void initParser(Parser* parser, Lexer* lexer);
ASTNode* parseProgram(Parser* parser);

#endif