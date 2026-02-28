#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    
    // Keywords
    TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR, TOKEN_TO,
    TOKEN_FUNC, TOKEN_RETURN, TOKEN_TRUE, TOKEN_FALSE, TOKEN_NULL,
    
    // Types (New)
    TOKEN_NUM_TYPE,
    TOKEN_STR_TYPE,
    TOKEN_ARR_TYPE,
    
    // Operators
    TOKEN_VARDECL, // :=
    TOKEN_ASSIGN,  // =
    TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_GT, TOKEN_LE, TOKEN_GE,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT,
    TOKEN_PLUS_ASSIGN, TOKEN_MINUS_ASSIGN,
    TOKEN_AND, TOKEN_OR, TOKEN_NOT,
    
    // Symbols
    TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_LBRACKET, TOKEN_RBRACKET,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_SEMICOLON, TOKEN_COLON
    
} TokenType;

typedef struct {
    TokenType type;
    char value[100];
} Token;

typedef struct {
    const char* src;
    int pos;
    int length;
    int line;
} Lexer;

void initLexer(Lexer* lexer, const char* src);
Token getNextToken(Lexer* lexer);

#endif