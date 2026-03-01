#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Helpers ---

void nextToken(Parser* parser) {
    if (parser->nextToken.type != TOKEN_EOF) {
        parser->currentToken = parser->nextToken;
        parser->nextToken = (Token){TOKEN_EOF, ""};
    } else {
        parser->currentToken = getNextToken(parser->lexer);
    }
}

Token peekNextToken(Parser* parser) {
    if (parser->nextToken.type == TOKEN_EOF) {
        parser->nextToken = getNextToken(parser->lexer);
    }
    return parser->nextToken;
}

void expect(Parser* parser, TokenType type) {
    if (parser->currentToken.type != type) {
        printf("PARSE ERROR: Expected token %d but got %d (%s) on line %d\n", 
               type, parser->currentToken.type, parser->currentToken.value, parser->lexer->line);
        exit(1);
    }
    nextToken(parser);
}

ASTNode* createNode(ASTNodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->left = NULL; node->right = NULL;
    node->next = NULL; node->body = NULL; node->step = NULL;
    node->name[0] = '\0'; node->value[0] = '\0';
    return node;
}

// Forward declarations
ASTNode* parseStatement(Parser* parser);
ASTNode* parseExpression(Parser* parser);
ASTNode* parseTerm(Parser* parser);
ASTNode* parseFactor(Parser* parser);
ASTNode* parseBlock(Parser* parser);

// --- Entry Point ---

void initParser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->nextToken = (Token){TOKEN_EOF, ""};
    nextToken(parser);
}

ASTNode* parseProgram(Parser* parser) {
    ASTNode* root = createNode(AST_BLOCK);
    root->left = NULL;
    ASTNode* last = NULL;

    while (parser->currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(parser);
        if (stmt) {
            if (!root->left) root->left = stmt;
            else last->next = stmt;
            last = stmt;
        }
    }
    return root;
}

// --- Statement Parsing ---

ASTNode* parseStatement(Parser* parser) {
    Token t = parser->currentToken;
    
    // 1. Explicit Definition: Num x = 5
    if (t.type == TOKEN_NUM_TYPE || t.type == TOKEN_STR_TYPE || t.type == TOKEN_ARR_TYPE) {
        nextToken(parser); // Eat type
        
        if (parser->currentToken.type != TOKEN_IDENTIFIER) {
            printf("Error: Expected variable name after type on line %d\n", parser->lexer->line);
            exit(1);
        }
        
        ASTNode* node = createNode(AST_VARDECL);
        strcpy(node->name, parser->currentToken.value);
        nextToken(parser); // Eat identifier
        
        if (parser->currentToken.type != TOKEN_ASSIGN && parser->currentToken.type != TOKEN_VARDECL) {
            printf("Error: Expected '=' after variable name on line %d\n", parser->lexer->line);
            exit(1);
        }
        nextToken(parser); // Eat '='
        
        node->right = parseExpression(parser);
        return node;
    }
    
    // 2. Standard Declaration: x := 5
    if (t.type == TOKEN_IDENTIFIER && peekNextToken(parser).type == TOKEN_VARDECL) {
        ASTNode* node = createNode(AST_VARDECL);
        strcpy(node->name, t.value);
        nextToken(parser); // Eat identifier
        nextToken(parser); // Eat :=
        node->right = parseExpression(parser);
        return node;
    }
    
    // 3. Assignment: x = 5
    if (t.type == TOKEN_IDENTIFIER && peekNextToken(parser).type == TOKEN_ASSIGN) {
        ASTNode* node = createNode(AST_ASSIGN);
        strcpy(node->name, t.value);
        nextToken(parser); // Eat identifier
        nextToken(parser); // Eat =
        node->right = parseExpression(parser);
        return node;
    }
    
    // 4. Keywords
    if (t.type == TOKEN_IF) {
        nextToken(parser); 
        ASTNode* node = createNode(AST_IF);
        expect(parser, TOKEN_LPAREN);
        node->left = parseExpression(parser);
        expect(parser, TOKEN_RPAREN);
        node->right = parseBlock(parser);
        if (parser->currentToken.type == TOKEN_ELSE) {
            nextToken(parser);
            node->body = parseBlock(parser);
        }
        return node;
    }
    
    if (t.type == TOKEN_WHILE) {
        nextToken(parser);
        ASTNode* node = createNode(AST_WHILE);
        expect(parser, TOKEN_LPAREN);
        node->left = parseExpression(parser);
        expect(parser, TOKEN_RPAREN);
        node->right = parseBlock(parser);
        return node;
    }
    
    if (t.type == TOKEN_FOR) {
        nextToken(parser);
        ASTNode* node = createNode(AST_FOR);
        // Assuming syntax: for i := start to end { }
        if (parser->currentToken.type == TOKEN_IDENTIFIER) {
             strcpy(node->name, parser->currentToken.value);
             nextToken(parser); 
             expect(parser, TOKEN_VARDECL);
        }
        node->left = parseExpression(parser); // Start
        expect(parser, TOKEN_TO);
        node->right = parseExpression(parser); // End
        node->body = parseBlock(parser);
        return node;
    }
    
    if (t.type == TOKEN_FUNC) {
        nextToken(parser);
        ASTNode* node = createNode(AST_FUNCDECL);
        if (parser->currentToken.type != TOKEN_IDENTIFIER) { printf("Error: Expected func name\n"); exit(1); }
        strcpy(node->name, parser->currentToken.value);
        nextToken(parser);
        expect(parser, TOKEN_LPAREN);
        
        ASTNode* lastArg = NULL;
        while (parser->currentToken.type != TOKEN_RPAREN) {
            ASTNode* arg = createNode(AST_IDENTIFIER);
            strcpy(arg->name, parser->currentToken.value);
            nextToken(parser);
            if (parser->currentToken.type == TOKEN_COMMA) nextToken(parser);
            
            if (!node->left) node->left = arg;
            else lastArg->next = arg;
            lastArg = arg;
        }
        expect(parser, TOKEN_RPAREN);
        node->right = parseBlock(parser);
        return node;
    }
    
    if (t.type == TOKEN_RETURN) {
        nextToken(parser);
        ASTNode* node = createNode(AST_RETURN);
        if (parser->currentToken.type != TOKEN_SEMICOLON && parser->currentToken.type != TOKEN_RBRACE) {
            node->left = parseExpression(parser);
        }
        return node;
    }
    
    // 5. Expression Statement
    return parseExpression(parser);
}

// --- Expression Parsing ---

ASTNode* parseExpression(Parser* parser) {
    ASTNode* left = parseTerm(parser);
    
    while (parser->currentToken.type == TOKEN_PLUS || parser->currentToken.type == TOKEN_MINUS) {
        ASTNode* node = createNode(AST_BINOP);
        strcpy(node->name, parser->currentToken.value);
        nextToken(parser);
        node->left = left;
        node->right = parseTerm(parser);
        left = node;
    }
    return left;
}

ASTNode* parseTerm(Parser* parser) {
    ASTNode* left = parseFactor(parser);
    
    while (parser->currentToken.type == TOKEN_STAR || parser->currentToken.type == TOKEN_SLASH || parser->currentToken.type == TOKEN_PERCENT) {
        ASTNode* node = createNode(AST_BINOP);
        strcpy(node->name, parser->currentToken.value);
        nextToken(parser);
        node->left = left;
        node->right = parseFactor(parser);
        left = node;
    }
    return left;
}

ASTNode* parseFactor(Parser* parser) {
    Token t = parser->currentToken;
    
    if (t.type == TOKEN_NUMBER) {
        ASTNode* node = createNode(AST_NUMBER);
        strcpy(node->value, t.value);
        nextToken(parser);
        return node;
    }
    if (t.type == TOKEN_STRING) {
        ASTNode* node = createNode(AST_STRING);
        strcpy(node->value, t.value);
        nextToken(parser);
        return node;
    }
    if (t.type == TOKEN_TRUE || t.type == TOKEN_FALSE) {
        ASTNode* node = createNode(AST_BOOL);
        strcpy(node->value, t.type == TOKEN_TRUE ? "1" : "0");
        nextToken(parser);
        return node;
    }
    if (t.type == TOKEN_NULL) {
        ASTNode* node = createNode(AST_NULL);
        nextToken(parser);
        return node;
    }
    
    // Array Literal
    if (t.type == TOKEN_LBRACKET) {
        nextToken(parser);
        ASTNode* node = createNode(AST_ARRAY);
        node->left = NULL;
        ASTNode* last = NULL;
        
        while (parser->currentToken.type != TOKEN_RBRACKET) {
            ASTNode* item = parseExpression(parser);
            if (!node->left) node->left = item;
            else last->next = item;
            last = item;
            
            if (parser->currentToken.type == TOKEN_COMMA) nextToken(parser);
        }
        expect(parser, TOKEN_RBRACKET);
        return node;
    }
    
    // Parentheses
    if (t.type == TOKEN_LPAREN) {
        nextToken(parser);
        ASTNode* node = parseExpression(parser);
        expect(parser, TOKEN_RPAREN);
        return node;
    }
    
    // Identifiers / Calls / Keywords
    if (t.type == TOKEN_IDENTIFIER || t.type == TOKEN_STR_TYPE || t.type == TOKEN_NUM_TYPE || t.type == TOKEN_ARR_TYPE) {
        // Handle built-in calls: Str(), Num(), Array()
        if (t.type == TOKEN_STR_TYPE || t.type == TOKEN_NUM_TYPE || t.type == TOKEN_ARR_TYPE) {
            if (peekNextToken(parser).type == TOKEN_LPAREN) {
                 ASTNode* node = createNode(AST_CALL);
                 if (t.type == TOKEN_STR_TYPE) strcpy(node->name, "Str");
                 else if (t.type == TOKEN_NUM_TYPE) strcpy(node->name, "Num");
                 else strcpy(node->name, "Array");
                 
                 nextToken(parser); // Eat keyword
                 nextToken(parser); // Eat (
                 if (parser->currentToken.type != TOKEN_RPAREN) {
                     node->left = parseExpression(parser);
                 }
                 expect(parser, TOKEN_RPAREN);
                 return node;
            }
            printf("Error: Unexpected keyword '%s'\n", t.value);
            exit(1);
        }
        
        // Standard Identifier or Method Call
        ASTNode* node = createNode(AST_IDENTIFIER);
        strcpy(node->name, t.value);
        nextToken(parser);
        
        // NEW: Handle Method Chaining (e.g., console.typeln)
        while (parser->currentToken.type == TOKEN_DOT) {
            strcat(node->name, "."); // Append dot to name
            nextToken(parser); // Eat dot
            
            if (parser->currentToken.type != TOKEN_IDENTIFIER) {
                printf("Error: Expected identifier after '.' on line %d\n", parser->lexer->line);
                exit(1);
            }
            strcat(node->name, parser->currentToken.value); // Append next part
            nextToken(parser); // Eat next part
        }
        
        // Function Call
        if (parser->currentToken.type == TOKEN_LPAREN) {
            node->type = AST_CALL;
            nextToken(parser);
            if (parser->currentToken.type != TOKEN_RPAREN) {
                node->left = parseExpression(parser);
            }
            expect(parser, TOKEN_RPAREN);
        }
        
        return node;
    }
    
    printf("Error: Unexpected token '%s' in expression on line %d\n", t.value, parser->lexer->line);
    exit(1);
}

ASTNode* parseBlock(Parser* parser) {
    expect(parser, TOKEN_LBRACE);
    ASTNode* block = createNode(AST_BLOCK);
    block->left = NULL;
    ASTNode* last = NULL;
    
    while (parser->currentToken.type != TOKEN_RBRACE && parser->currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(parser);
        if (stmt) {
            if (!block->left) block->left = stmt;
            else last->next = stmt;
            last = stmt;
        }
    }
    expect(parser, TOKEN_RBRACE);
    return block;
}