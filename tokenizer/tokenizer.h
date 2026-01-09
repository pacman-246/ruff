#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

typedef enum {
    TOKEN_EOF = 0,

    // インデント
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,

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
    TOKEN_IN, // in
    TOKEN_RETURN, // return
    TOKEN_BREAK, // break
    TOKEN_CONTINUE, // continue
    TOKEN_PASS, // pass
    TOKEN_FUNC, // func
    TOKEN_VAR, // var
    TOKEN_LET, // let

    // 理論値
    TOKEN_TRUE, // true
    TOKEN_FALSE, // false

    // 記号（優先度順）
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_DOT, // .
    TOKEN_STAR, // *
    TOKEN_SLASH, // /
    TOKEN_PERCENT, // %
    TOKEN_PLUS, // +
    TOKEN_MINUS, // -
    TOKEN_LT, // <
    TOKEN_LE, // <=
    TOKEN_GT, // >
    TOKEN_GE, // >=
    TOKEN_EQ, // ==
    TOKEN_NE, // !=
    TOKEN_ARROW, // ->
    TOKEN_COLON, // :
    TOKEN_COMMA, // ,
    TOKEN_ASSIGN, // =

} TokenType;

typedef struct {
    TokenType type;
    char *value;
    // エラーのときエラーが起きている行を特定できるようになる
    int pos;
} Token;

typedef struct {
    Token *tokens;
    size_t count; // トークン総数
    size_t capacity; // 確保容量
    size_t pos; // 現在位置
} TokenList;

TokenList tokenizer(char *code);
static void addToken(TokenList *list, TokenType type, const char *value, int pos);
TokenType checkTokenType(char *ident);
Token *peekToken(TokenList *list);
Token *nextToken(TokenList *list);
void freeTokenList(TokenList *list);
const char *tokenTypeToString(TokenType type);
void printTokens(const TokenList *list);

#endif
