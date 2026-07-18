#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "tokenizer.h"
#include "../arena/arena.h"

TokenList *tokenizer(char *code, Arena *arena) {
    // tokenListの初期化
    TokenList *tokenList = arenaAlloc(arena, sizeof(TokenList));
    tokenList->tokens = NULL;
    tokenList->count = 0;
    tokenList->capacity = 0;
    tokenList->pos = 0;

    int nowLine = 1;

    size_t i = 0;

    while (code[i] != '\0') {
        char c = code[i];
        switch (c) {
            case ' ': // 空白
                i++;
                break;

            case '\n': // 改行
                nowLine++;
                i++;
                break;

            case '"': // 文字列
                // 一時的な文字列構築バッファ。最終的にarenaStrdupでアリーナにコピーする
                char *str = malloc(2);
                str[0] = '"';
                str[1] = '\0';

                i++; // 最初の"

                while (code[i] != '"' && code[i] != '\0') {
                    size_t len = strlen(str); // \0は含まれないので以下のコードでも成り立つ
                    str = realloc(str, len + 2);

                    str[len] = code[i];
                    str[len + 1] = '\0';
                    i++;
                }

                size_t len = strlen(str);
                str = realloc(str, len + 2);

                str[len] = code[i];
                str[len + 1] = '\0';

                // 最後の"
                i++;

                addToken(tokenList, TOKEN_STRING, str, nowLine, arena);
                free(str);
                break;

            case '-':
                i++; // -を消費
                if (code[i] != '>') {
                    printf("OperatorError: Line-%d\nThe only operator that starts with - is ->\n",
                            nowLine);
                    exit(1);
                }
                i++; // >を消費

                addToken(tokenList, TOKEN_ARROW, "->", nowLine, arena);
                break;

            case ',':
                i++; // ,を消費
                addToken(tokenList, TOKEN_COMMA, ",", nowLine, arena);
                break;

            case ';':
                i++; // ;を消費
                addToken(tokenList, TOKEN_SEMICOLON, ";", nowLine, arena);
                break;

            case '(':
                i++; // (を消費
                addToken(tokenList, TOKEN_LPAREN, "(", nowLine, arena);
                break;

            case ')':
                i++; // )を消費
                addToken(tokenList, TOKEN_RPAREN, ")", nowLine, arena);
                break;

            default:
                if (c >= '0' && c <= '9') { // 数字
                    char *num = malloc(2);
                    num[0] = c;
                    num[1] = '\0';

                    i++; // 数字の最初を消費

                    int dot = 0;

                    while (((code[i] >= '0' && code[i] <= '9') || code[i] == '.') && code[i] != '\0') {
                        if (code[i] == '.') {
                            if (dot == 0) {
                                dot = 1;
                            } else {
                                printf("NumberError: Line-%d\nYou cannot include a period (.) more than once in a number\n",
                                    nowLine);
                                exit(1);
                            }
                        }

                        size_t len = strlen(num);
                        num = realloc(num, len + 2);
                        num[len] = code[i];
                        num[len + 1] = '\0';
                        i++;
                    }

                    if (dot) {
                        addToken(tokenList, TOKEN_FLOAT, num, nowLine, arena);
                    } else {
                        addToken(tokenList, TOKEN_INT, num, nowLine, arena);
                    }

                    free(num);

                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') { // 識別子、キーワード
                    char *ident = malloc(2);
                    ident[0] = c;
                    ident[1] = '\0';

                    i++; //識別子、キーワードの最初の文字

                    while (((code[i] >= 'a' && code[i] <= 'z') || (code[i] >= 'A' && code[i] <= 'Z') || code[i] == '_') && code[i] != '\0') {
                        size_t len = strlen(ident);
                        ident = realloc(ident, len + 2); // 文字 + '\0'
                        ident[len] = code[i];
                        ident[len + 1] = '\0';
                        i++;
                    }

                    if (strcmp(ident, "true") == 0) {
                        addToken(tokenList, TOKEN_BOOL, "true", nowLine, arena);
                    } else if (strcmp(ident, "false") == 0) {
                        addToken(tokenList, TOKEN_BOOL, "false", nowLine, arena);
                    } else {
                        addToken(tokenList, TOKEN_IDENT, ident, nowLine, arena);
                    }

                    free(ident);
                }
        }
    }

    // ファイルの終わりを明示する
    addToken(tokenList, TOKEN_EOF, "\0", nowLine, arena);

    return tokenList;
}

// トークンを追加
static void addToken(TokenList *list, TokenType type, const char *value, int pos, Arena *arena) {
    if (list->count >= list->capacity) {
        size_t old_capacity = list->capacity;
        list->capacity = list->capacity ? list->capacity * 2 : 8;
        Token *new_tokens = arenaAlloc(arena, sizeof(Token) * list->capacity);
        if (list->tokens && old_capacity > 0) {
            memcpy(new_tokens, list->tokens, sizeof(Token) * old_capacity);
        }
        list->tokens = new_tokens;
    }

    Token *t = &list->tokens[list->count++];
    t->type = type;
    t->value = value ? arenaStrdup(arena, value) : NULL;
    t->pos = pos;
}

// トークンリストを表示
void printTokenList(TokenList *list) {
    for (size_t i = 0; i < list->count; i++) {
        printf("%zu: type=%d value=%s\n",
           list->tokens[i].pos,
           list->tokens[i].type,
           list->tokens[i].value);
    }
}

// parserで使う関数

// 現在の位置のトークンを取得
Token *peekToken(TokenList *list) {
    return &list->tokens[list->pos];
}

// 位置を進め、そこのトークンを取得
Token *nextToken(TokenList *list) {
    return &list->tokens[++list->pos];
}
