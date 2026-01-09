#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tokenizer.h"
#include "../utils/utils.h"

TokenList tokenizer(char *code) {
    TokenList list = {0};
    size_t i = 0;
    int nowIndentCount = 0;
    int lastIndentCount = 0;
    int nowLine = 1;
    // 一行目の最初も行の最初として扱ってほしいためtrue
    bool afterNewline = true;
    bool inParen = false;

    while (code[i] != '\0') {
        char c = code[i];

        // 行の最初尚且つ括弧の中じゃなかった場合はインデントの処理をする
        if (afterNewline && !inParen) {
            if (c == ' ') {
                // 最初の空白を飛ばす
                i++;
                int count = 1;
                while ((code[i] == ' ') && code[i] != '\0') {
                    count++;
                    i++;
                }

                if ((count % 4) == 0) {
                    nowIndentCount = (count / 4);
                }

            }

            char str[20];
            if (nowIndentCount != lastIndentCount) {
                sprintf(str, "%d", nowIndentCount);
            }

            // インデントが増えている場合はINDENTを追加する
            // インデントが減っている場合はDEDENTを追加する
            if (nowIndentCount > lastIndentCount) {
                for (int j = 1; j <= (nowIndentCount - lastIndentCount); j++) {
                    if (j == 1) {
                        addToken(&list, TOKEN_INDENT, str, nowLine);
                    } else {
                        errorAtPos("multiple indents have suddenly increased at once", nowLine, code);
                    }
                }
            } else if (nowIndentCount < lastIndentCount) {
                for (int j = 1; j <= (lastIndentCount - nowIndentCount); j++) {
                    if (j == 1) {
                        addToken(&list, TOKEN_DEDENT, str, nowLine);
                    } else {
                        nextToken(&list);
                    }
                }
            }
        }

        switch (c) {
        // インデント
        case '\r':
            if (inParen) {
                i++;
                break;
            }

            if (code[i + 1] == '\n') {
                i++; // '\n' をスキップ
            }
            // printTokensで改行されないように\\n
            addToken(&list, TOKEN_NEWLINE, "\\n", nowLine);
            lastIndentCount = nowIndentCount;
            nowIndentCount = 0;
            nowLine++;
            i++;
            break;
        
        case '\n':
            if (inParen) {
                i++;
                break;
            }

            // printTokensで改行されないように\\n
            addToken(&list, TOKEN_NEWLINE, "\\n", nowLine);
            lastIndentCount = nowIndentCount;
            nowIndentCount = 0;
            nowLine++;
            i++;
            break;

        case ' ':
            // 行の最初だった場合ももう処理されてるので何もしない
            // 行の途中または括弧の中だった場合は飛ばす
            if (!afterNewline || inParen) {
                i++;
            }

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

            addToken(&list, TOKEN_STRING, str, nowLine);
            break;

        //記号
        case '+':
            addToken(&list, TOKEN_PLUS, "+", nowLine);
            i++;
            break;

        case '-':
            if (code[i + 1] == '>') {
                addToken(&list, TOKEN_ARROW, "->", nowLine);
                i += 2; // -と>を消費

            } else {
                addToken(&list, TOKEN_MINUS, "-", nowLine);
                i++;
            }
            break;

        case '*':
            addToken(&list, TOKEN_STAR, "*", nowLine);
            i++;
            break;

        case '/':
            addToken(&list, TOKEN_SLASH, "/", nowLine);
            i++;
            break;

        case '%':
            addToken(&list, TOKEN_PERCENT, "%", nowLine);
            i++;
            break;

        case '(':
            addToken(&list, TOKEN_LPAREN, "(", nowLine);
            i++;
            inParen = true;
            break;

        case ')':
            addToken(&list, TOKEN_RPAREN, ")", nowLine);
            i++;
            inParen = false;
            break;

        case ':':
            addToken(&list, TOKEN_COLON, ":", nowLine);
            i++;
            break;

        case ',':
            addToken(&list, TOKEN_COMMA, ",", nowLine);
            i++;
            break;

        case '<':
            i++;
            if (code[i] == '=') {
                i++;
                addToken(&list, TOKEN_LE, "<=", nowLine);
            } else {
                addToken(&list, TOKEN_LT, "<", nowLine);
            }
            break;

        case '>':
            i++;
            if (code[i] == '=') {
                i++;
                addToken(&list, TOKEN_GE, ">=", nowLine);
            } else {
                addToken(&list, TOKEN_GT, ">", nowLine);
            }
            break;

        case '=':
            i++;
            if (code[i] == '=') {
                i++;
                addToken(&list, TOKEN_EQ, "==", nowLine);
            } else {
                addToken(&list, TOKEN_ASSIGN, "=", nowLine);
            }
            break;

        case '!':
            i++;
            if (code[i] == '=') {
                i++;
                addToken(&list, TOKEN_NE, "!=", nowLine);
            } else {
                // 許容されていない文字
                printf("during lexical analysis, an invalid character was detected\ncharacter: !\n");
            }
            break;

        case '.':
            addToken(&list, TOKEN_DOT, ".", nowLine);
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

                addToken(&list, TOKEN_NUMBER, num, nowLine);

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

                addToken(&list, tt, ident, nowLine);

            } else { // 許容されていない文字
                char msg[128];
                snprintf(msg, sizeof(msg), 
                    "during lexical analysis, an invalid character was detected\ncharacter: %c", c);
                errorAtPos(msg, nowLine, code);
            }
            break;
        }

        if (c == '\n' || c == '\r') {
            afterNewline = true;
        } else {
            afterNewline = false;
        }
    }

    addToken(&list, TOKEN_EOF, NULL, nowLine);
    return list;
}

// トークンを追加
static void addToken(TokenList *list, TokenType type, const char *value, int pos) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 8;
        list->tokens = realloc(list->tokens,
            sizeof(Token) * list->capacity);
    }

    Token *t = &list->tokens[list->count++];
    t->type = type;
    t->value = value ? strdup(value) : NULL;
    t->pos = pos;
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
    } else if (strcmp(ident, "in") == 0) {
        return TOKEN_IN;
    } else if (strcmp(ident, "return") == 0) {
        return TOKEN_RETURN;
    } else if (strcmp(ident, "break") == 0) {
        return TOKEN_BREAK;
    } else if (strcmp(ident, "continue") == 0) {
        return TOKEN_CONTINUE;
    } else if (strcmp(ident, "pass") == 0) {
        return TOKEN_PASS;
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
    // 次のトークンを返したいため後置インクリメント
    return &list->tokens[++list->pos];
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

        // インデント
        case TOKEN_NEWLINE:  return "NEWLINE";
        case TOKEN_INDENT:   return "INDENT";
        case TOKEN_DEDENT:   return "DEDENT";

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
        case TOKEN_IN:       return "IN";
        case TOKEN_RETURN:   return "RETURN";
        case TOKEN_BREAK:    return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
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
        case TOKEN_PERCENT:  return "PERCENT"; 
        case TOKEN_LPAREN:   return "LPAREN";
        case TOKEN_RPAREN:   return "RPAREN";
        case TOKEN_COLON:    return "COLON";
        case TOKEN_ARROW:    return "ARROW";
        case TOKEN_COMMA:    return "COMMA";
        case TOKEN_ASSIGN:   return "ASSIGN";
        case TOKEN_DOT:      return "DOT";
        case TOKEN_LT:       return "LT";
        case TOKEN_LE:       return "LE";
        case TOKEN_GT:       return "GT";
        case TOKEN_GE:       return "GE";
        case TOKEN_EQ:       return "EQ";
        case TOKEN_NE:       return "NE";

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
