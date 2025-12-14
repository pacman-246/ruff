#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

typedef enum {
    TOKEN_EOF = 0,

    // リテラル
    TOKEN_IDENT, // a
    TOKEN_NUMBER, // 1
    TOKEN_STRING, // "a"

    // キーワード
    TOKEN_IF, // if
    TOKEN_ELIF, // elif
    TOKEN_ELSE, // else
    TOKEN_WHILE, // while
    TOKEN_FOR, // for
    TOKEN_RETURN, // return
    TOKEN_PASS, // pass
    TOKEN_FUNC, // func
    TOKEN_VAR, // var
    TOKEN_LET, // let

    // 理論値
    TOKEN_TRUE, // true
    TOKEN_FALSE, // false

    // 記号
    TOKEN_PLUS, // +
    TOKEN_MINUS, // -
    TOKEN_STAR, // *
    TOKEN_SLASH, // /
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_COLON, // :
    TOKEN_ARROW, // ->
    TOKEN_COMMA, // ,
    TOKEN_ASSIGN, // =

} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

typedef struct {
    Token *tokens;
    size_t count; // トークン総数
    size_t capacity; // 確保容量
    size_t pos; // 現在位置
} TokenList;

TokenList tokenizer(char *code);
static void addToken(TokenList *list, TokenType type, const char *value);
TokenType checkTokenType(char *ident);
Token *peekToken(TokenList *list);
Token *nextToken(TokenList *list);
void freeTokenList(TokenList *list);
const char *tokenTypeToString(TokenType type);
void printTokens(const TokenList *list);

#endif
