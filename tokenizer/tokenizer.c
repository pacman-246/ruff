#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

TokenList tokenizer(char *code) {
    TokenList list = {0};
    size_t i = 0;

    while (code[i] != '\0') {
        char c = code[i];

        switch (c) {
        //空白
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            i++;
            break;

        //文字列
        case '"':
            char *str = malloc(2);
            str[0] = '"';
            str[1] = '\0';

            i++; // 最初の "

            while (code[i] != '"' && code[i] != '\0') {
                size_t len = strlen(str);
                str = realloc(str, len + 2); // 文字 + '\0'
                str[len] = code[i];
                str[len + 1] = '\0';
                i++;
            }

            // 閉じの " を入れる
            size_t len = strlen(str);
            str = realloc(str, len + 2); // " + '\0'
            str[len] = code[i];
            str[len + 1] = '\0';

            // 閉じの "
            i++;

            addToken(&list, TOKEN_STRING, str);
            break;

        //記号
        case '+':
            addToken(&list, TOKEN_PLUS, "+");
            i++;
            break;

        case '-':
            if (code[i + 1] == '>') {
                addToken(&list, TOKEN_ARROW, "->");
                i += 2; // -と>を消費

            } else {
                addToken(&list, TOKEN_MINUS, "-");
                i++;
            }
            break;

        case '*':
            addToken(&list, TOKEN_STAR, "*");
            i++;
            break;

        case '/':
            addToken(&list, TOKEN_SLASH, "/");
            i++;
            break;

        case '(':
            addToken(&list, TOKEN_LPAREN, "(");
            i++;
            break;

        case ')':
            addToken(&list, TOKEN_RPAREN, ")");
            i++;
            break;

        case ':':
            addToken(&list, TOKEN_COLON, ":");
            i++;
            break;

        case ',':
            addToken(&list, TOKEN_COMMA, ",");
            i++;
            break;

        case '=':
            addToken(&list, TOKEN_ASSIGN, "=");
            i++;
            break;
        
        default:
            if (c >= '0' && c <= '9') { // 数字
                char *num = malloc(2);
                num[0] = c;
                num[1] = '\0';

                i++; // 数字の最初の文字

                while (((code[i] >= '0' && code[i] <= '9') || code[i] == '.') && code[i] != '\0') {
                    size_t len = strlen(num);
                    num = realloc(num, len + 2); // 数字 + '\0'
                    num[len] = code[i];
                    num[len + 1] = '\0';
                    i++;
                }

                addToken(&list, TOKEN_NUMBER, num);

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

                //キーワードや識別子を判断してTokenTypeを取得する
                TokenType tt = checkTokenType(ident);

                addToken(&list, tt, ident);

            } else { // 許容されていない文字
                printf("during lexical analysis, an invalid character was detected\ncharacter: %c\n", c);
                exit(1);
            }
            break;
        }
    }

    addToken(&list, TOKEN_EOF, NULL);
    return list;
}

// トークンを追加
static void addToken(TokenList *list, TokenType type, const char *value) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 8;
        list->tokens = realloc(list->tokens,
            sizeof(Token) * list->capacity);
    }

    Token *t = &list->tokens[list->count++];
    t->type = type;
    t->value = value ? strdup(value) : NULL;
}

// 識別子かキーワードのときTokenTypeは何か、を返す
TokenType checkTokenType(char *ident) {
    if (strcmp(ident, "if") == 0) {
        return TOKEN_IF;
    } else if (strcmp(ident, "elif") == 0) {
        return TOKEN_ELIF;
    } else if (strcmp(ident, "else") == 0) {
        return TOKEN_ELSE;
    } else if (strcmp(ident, "while") == 0) {
        return TOKEN_WHILE;
    } else if (strcmp(ident, "for") == 0) {
        return TOKEN_FOR;
    } else if (strcmp(ident, "return") == 0) {
        return TOKEN_RETURN;
    } else if (strcmp(ident, "func") == 0) {
        return TOKEN_FUNC;
    } else if (strcmp(ident, "var") == 0) {
        return TOKEN_VAR;
    } else if (strcmp(ident, "let") == 0) {
        return TOKEN_LET;
    } else if (strcmp(ident, "true") == 0) {
        return TOKEN_TRUE;
    } else if (strcmp(ident, "false") == 0) {
        return TOKEN_FALSE;
    } else {
        return TOKEN_IDENT;
    }
}

// 現在位置のトークンを取得
Token *peekToken(TokenList *list) {
    if (list->pos >= list->count) {
        return NULL;
    }
    return &list->tokens[list->pos];
}

// posを進めつつ取得
Token *nextToken(TokenList *list) {
    if (list->pos >= list->count) {
        return NULL;
    }
    return &list->tokens[list->pos++];
}

// TokenListをfreeする
void freeTokenList(TokenList *list) {
    if (!list || !list->tokens) return;

    for (size_t i = 0; i < list->count; i++) {
        free(list->tokens[i].value);  // NULLでも安全
    }

    free(list->tokens);

    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
    list->pos = 0;
}

// tokentypeを文字列に
const char *tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_EOF:      return "EOF";

        // リテラル
        case TOKEN_IDENT:    return "IDENT";
        case TOKEN_NUMBER:   return "NUMBER";
        case TOKEN_STRING:   return "STRING";

        // キーワード
        case TOKEN_IF:       return "IF";
        case TOKEN_ELIF:     return "ELIF";
        case TOKEN_ELSE:     return "ELSE";
        case TOKEN_WHILE:    return "WHILE";
        case TOKEN_FOR:      return "FOR";
        case TOKEN_RETURN:   return "RETURN";
        case TOKEN_PASS:     return "PASS";
        case TOKEN_FUNC:     return "FUNC";
        case TOKEN_VAR:      return "VAR";
        case TOKEN_LET:      return "LET";

        // 論理値
        case TOKEN_TRUE:     return "TRUE";
        case TOKEN_FALSE:    return "FALSE";

        // 記号
        case TOKEN_PLUS:     return "PLUS";
        case TOKEN_MINUS:    return "MINUS";
        case TOKEN_STAR:     return "STAR";
        case TOKEN_SLASH:    return "SLASH";
        case TOKEN_LPAREN:   return "LPAREN";
        case TOKEN_RPAREN:   return "RPAREN";
        case TOKEN_COLON:    return "COLON";
        case TOKEN_ARROW:    return "ARROW";
        case TOKEN_COMMA:    return "COMMA";
        case TOKEN_ASSIGN:   return "ASSIGN";

        default:             return "UNKNOWN";
    }
}

// TokenListを表示
void printTokens(const TokenList *list) {
    printf("====tokens====\n");

    for (size_t i = 0; i < list->count; i++) {
        const Token *t = &list->tokens[i];

        printf("[%zu] %s", i, tokenTypeToString(t->type));

        if (t->value) {
            printf(" \"%s\"", t->value);
        }

        printf(" | ");
    }

    printf("\n========\n");
}