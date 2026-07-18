#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <stdlib.h>
#include "../arena/arena.h"

// トークンの種類
typedef enum {
    TOKEN_EOF = 0, // ファイルの終わり

    // リテラル
    TOKEN_IDENT, // 識別子 1
    TOKEN_INT, // 整数 2
    TOKEN_FLOAT, // 浮動小数点数 3
    TOKEN_STRING, // 文字列 4
    TOKEN_BOOL, // bool 5

    // 記号
    TOKEN_ARROW, // -> 6
    TOKEN_COMMA, // , 7
    TOKEN_SEMICOLON, // ; 8
    TOKEN_LPAREN, // ( 9
    TOKEN_RPAREN, // ) 10
} TokenType;

// トークン
typedef struct {
    TokenType type;
    char *value;
    // エラーのときエラーが起きている行を特定できるようになる
    int pos;
} Token;

// トークンリスト
typedef struct {
    Token *tokens;
    size_t count; // トークン総数
    size_t capacity; // 確保容量
    size_t pos; // 現在位置
} TokenList;

TokenList *tokenizer(char *code, Arena *arena);
static void addToken(TokenList *list, TokenType type, const char *value, int pos, Arena *arena);
void printTokenList(TokenList *list);
Token *peekToken(TokenList *list);
Token *nextToken(TokenList *list);

#endif
