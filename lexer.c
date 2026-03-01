#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// --- Helper Functions ---

static char peek(Lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->src[lexer->pos];
}

static char peekNext(Lexer* lexer) {
    if (lexer->pos + 1 >= lexer->length) return '\0';
    return lexer->src[lexer->pos + 1];
}

static char advance(Lexer* lexer) {
    char c = peek(lexer);
    if (c != '\0') {
        lexer->pos++;
        if (c == '\n') lexer->line++;
    }
    return c;
}

void skipWhitespace(Lexer* lexer) {
    while (isspace(peek(lexer))) {
        advance(lexer);
    }
}

static int isAlpha(char c) { return isalpha(c) || c == '_'; }
static int isDigit(char c) { return isdigit(c); }

// --- Initialization ---

void initLexer(Lexer* lexer, const char* src) {
    lexer->src = src;
    lexer->pos = 0;
    lexer->length = strlen(src);
    lexer->line = 1; 

    // Skip BOM
    if (lexer->length >= 3 && 
        (unsigned char)src[0] == 0xEF && 
        (unsigned char)src[1] == 0xBB && 
        (unsigned char)src[2] == 0xBF) {
        lexer->pos = 3; 
    }
}

// --- Token Scanning ---

Token getNextToken(Lexer* lexer) {
    // FIX: Initialize token to zero to clear garbage values (no more '♥')
    Token t = {0}; 
    t.type = TOKEN_EOF; 
    
    skipWhitespace(lexer);
    
    char c = peek(lexer);
    
    if (c == '\0') { return t; }
    
    // Identifier or Keyword
    if (isAlpha(c)) {
        int i = 0;
        while (isAlpha(peek(lexer)) || isDigit(peek(lexer))) {
            if (i < 99) t.value[i++] = advance(lexer);
            else advance(lexer);
        }
        t.value[i] = '\0';
        
        // Check Keywords
        if (strcmp(t.value, "if") == 0) t.type = TOKEN_IF;
        else if (strcmp(t.value, "else") == 0) t.type = TOKEN_ELSE;
        else if (strcmp(t.value, "while") == 0) t.type = TOKEN_WHILE;
        else if (strcmp(t.value, "for") == 0) t.type = TOKEN_FOR;
        else if (strcmp(t.value, "to") == 0) t.type = TOKEN_TO;
        else if (strcmp(t.value, "func") == 0) t.type = TOKEN_FUNC;
        else if (strcmp(t.value, "return") == 0) t.type = TOKEN_RETURN;
        else if (strcmp(t.value, "true") == 0) t.type = TOKEN_TRUE;
        else if (strcmp(t.value, "false") == 0) t.type = TOKEN_FALSE;
        else if (strcmp(t.value, "null") == 0) t.type = TOKEN_NULL;
        
        // Type Keywords
        else if (strcmp(t.value, "Num") == 0) t.type = TOKEN_NUM_TYPE;
        else if (strcmp(t.value, "Str") == 0) t.type = TOKEN_STR_TYPE;
        else if (strcmp(t.value, "Array") == 0) t.type = TOKEN_ARR_TYPE;
        
        else t.type = TOKEN_IDENTIFIER;
        
        return t;
    }
    
    // Number
    if (isDigit(c) || (c == '.' && isDigit(peekNext(lexer)))) {
        int i = 0;
        while (isDigit(peek(lexer)) || peek(lexer) == '.') {
            if (i < 99) t.value[i++] = advance(lexer);
            else advance(lexer);
        }
        t.value[i] = '\0';
        t.type = TOKEN_NUMBER;
        return t;
    }
    
    // String
    if (c == '"') {
        advance(lexer); 
        int i = 0;
        while (peek(lexer) != '"' && peek(lexer) != '\0') {
            if (i < 99) t.value[i++] = advance(lexer);
            else advance(lexer);
        }
        t.value[i] = '\0';
        if (peek(lexer) == '"') advance(lexer);
        t.type = TOKEN_STRING;
        return t;
    }
    
    // Two-character operators
    if (c == ':') {
        advance(lexer);
        if (peek(lexer) == '=') { advance(lexer); t.type = TOKEN_VARDECL; strcpy(t.value, ":="); return t; }
        t.type = TOKEN_COLON; return t; 
    }
    if (c == '=') {
        advance(lexer);
        if (peek(lexer) == '=') { advance(lexer); t.type = TOKEN_EQ; strcpy(t.value, "=="); return t; }
        t.type = TOKEN_ASSIGN; strcpy(t.value, "="); return t;
    }
    if (c == '+') {
        advance(lexer);
        if (peek(lexer) == '=') { advance(lexer); t.type = TOKEN_PLUS_ASSIGN; strcpy(t.value, "+="); return t; }
        t.type = TOKEN_PLUS; strcpy(t.value, "+"); return t;
    }
    if (c == '-') {
        advance(lexer);
        if (peek(lexer) == '=') { advance(lexer); t.type = TOKEN_MINUS_ASSIGN; strcpy(t.value, "-="); return t; }
        t.type = TOKEN_MINUS; strcpy(t.value, "-"); return t;
    }
    if (c == '!') {
        advance(lexer);
        if (peek(lexer) == '=') { advance(lexer); t.type = TOKEN_NEQ; strcpy(t.value, "!="); return t; }
        t.type = TOKEN_NOT; strcpy(t.value, "!"); return t;
    }
    if (c == '<') {
        advance(lexer);
        if (peek(lexer) == '=') { advance(lexer); t.type = TOKEN_LE; strcpy(t.value, "<="); return t; }
        t.type = TOKEN_LT; strcpy(t.value, "<"); return t;
    }
    if (c == '>') {
        advance(lexer);
        if (peek(lexer) == '=') { advance(lexer); t.type = TOKEN_GE; strcpy(t.value, ">="); return t; }
        t.type = TOKEN_GT; strcpy(t.value, ">"); return t;
    }
    if (c == '&') {
        advance(lexer);
        if (peek(lexer) == '&') { advance(lexer); t.type = TOKEN_AND; strcpy(t.value, "&&"); return t; }
        t.type = TOKEN_EOF; return t;
    }
    if (c == '|') {
        advance(lexer);
        if (peek(lexer) == '|') { advance(lexer); t.type = TOKEN_OR; strcpy(t.value, "||"); return t; }
        t.type = TOKEN_EOF; return t;
    }
    if (c == '/') { 
        advance(lexer);
        if (peek(lexer) == '/') {
            while (peek(lexer) != '\n' && peek(lexer) != '\0') advance(lexer);
            return getNextToken(lexer);
        }
        t.type = TOKEN_SLASH;
        strcpy(t.value, "/");
        return t;
    }
    
    // Single-character tokens
    advance(lexer);
    switch(c) {
        case '(': t.type = TOKEN_LPAREN; break;
        case ')': t.type = TOKEN_RPAREN; break;
        case '{': t.type = TOKEN_LBRACE; break;
        case '}': t.type = TOKEN_RBRACE; break;
        case ',': t.type = TOKEN_COMMA; break;
        case '.': t.type = TOKEN_DOT; break;
        case ';': t.type = TOKEN_SEMICOLON; break;
        case '*': t.type = TOKEN_STAR; break;
        case '%': t.type = TOKEN_PERCENT; break;
        case '[': t.type = TOKEN_LBRACKET; break; 
        case ']': t.type = TOKEN_RBRACKET; break;
        default:
            printf("LEXER ERROR: Unknown character '%c' on line %d\n", c, lexer->line);
            exit(1);
    }
    return t;
}