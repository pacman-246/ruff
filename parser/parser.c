#include <stdio.h>
#include <string.h>
#include "parser.h"

Node *parser(TokenList *list, Arena *arena) {
    return program(list, arena);

    if (peekToken(list)->type != TOKEN_EOF) {
        printf("EOFError: Line-%d\nThere is no EOF\n",
            peekToken(list)->pos);
        exit(1);
    }
}

// 複数文
// <program> ::= <statement>*
Node *program(TokenList *list, Arena *arena) {
    Node *node = application(list, arena);

    while (peekToken(list)->type == TOKEN_SEMICOLON) {
        Token *t = nextToken(list);
        if (t->type == TOKEN_EOF) {
            break;
        }

        Node *right = application(list, arena);

        Node *newNode = arenaAlloc(arena, sizeof(Node));
        newNode->type = NODE_PROGRAM;
        newNode->pos = node->pos;
        newNode->as.prog.left = node;
        newNode->as.prog.right = right;

        node = newNode;
    }

    return node;
}

// <statement> ::= <expr> ";"
// <expr> ::= <application>

// <application> ::= <atom>
//     | <application> "->" <atom>
Node *application(TokenList *list, Arena *arena) {
    Node *node = atom(list, arena);

    while (peekToken(list)->type == TOKEN_ARROW) {
        Token *t = nextToken(list); // ->を消費
        if (t->type == TOKEN_EOF) {
            break;
        }

        Node *right = atom(list, arena);

        Node *newNode = arenaAlloc(arena, sizeof(Node));
        newNode->type = NODE_APP;
        newNode->pos = node->pos;
        newNode->as.app.func = node;
        newNode->as.app.arg = right;

        node = newNode;
    }

    return node;
}

// <atom> ::= <literal>
//          | <ident>
//          | <quote>
//          | "(" <application> ")"
Node *atom(TokenList *list, Arena *arena) {
    TokenType tt = peekToken(list)->type;
    switch (tt) {
        // <literal>
        case TOKEN_NUMBER:
        case TOKEN_STRING:
        case TOKEN_BOOL:
        case TOKEN_LBRACKET:
            return literal(list, arena);

        // <identifier>
        case TOKEN_IDENT:
            return identifier(list, arena);

        // <quote>
        case TOKEN_DOLLAR:
            nextToken(list);
            Node *content = atom(list, arena);

            Node *newNode = arenaAlloc(arena, sizeof(Node));
            newNode->type = NODE_QUOTE;
            newNode->as.quote.node = content;
            newNode->pos = peekToken(list)->pos;

            return newNode;

        // "(" <expr> ")"
        case TOKEN_LPAREN:
            nextToken(list); // (を消費
            if (peekToken(list)->type == TOKEN_RPAREN) {
                return NULL;
            }

            Node *newNode_ = application(list, arena);

            if (peekToken(list)->type != TOKEN_RPAREN) {
                printf("ParenError: Line-%d\nThere is no end of paren\n",
                    peekToken(list)->pos);
                exit(1);
            }
            nextToken(list); // )を消費

            return newNode_;

        default:
            break;
    }
}

// リテラル
Node *literal(TokenList *list, Arena *arena) {
    Node *newNode = arenaAlloc(arena, sizeof(Node));

    TokenType tt = peekToken(list)->type;
    if (tt != TOKEN_NUMBER
        && tt != TOKEN_STRING
        && tt != TOKEN_BOOL
        && tt != TOKEN_LBRACKET) {
        printf("LiteralError: Line-%d\nThe part that was supposed to be treated as a literal was not a literal\n",
                peekToken(list)->pos);
        exit(1);
    }

    newNode->type = NODE_LITERAL;
    newNode->pos = peekToken(list)->pos;
    struct Literal lit;

    char *value = peekToken(list)->value;

    if (tt == TOKEN_NUMBER) {
        lit.type = LITERAL_NUMBER;
        lit.numberLiteral = atof(value);

    } else if (tt == TOKEN_STRING) {
        lit.type = LITERAL_STRING;
        size_t len = strlen(value);
        char *result = arenaAlloc(arena, len - 1);
        memcpy(result, value + 1, len - 2);
        result[len - 2] = '\0';

        lit.stringLiteral = result;

    } else if (tt == TOKEN_BOOL) {
        lit.type = LITERAL_BOOL;
        if (strcmp(value, "true") == 0) {
            lit.boolLiteral = true;
        } else if (strcmp(value, "false") == 0) {
            lit.boolLiteral = false;
        }

    } else if (tt == TOKEN_LBRACKET) {
        nextToken(list);
        lit.type = LITERAL_LIST;

        if (peekToken(list)->type == TOKEN_RBRACKET) {
            lit.listLiteral = NULL;
        } else {
            lit.listLiteral = comma(list, arena);
            if (peekToken(list)->type != TOKEN_RBRACKET) {
                printf("BracketError: Line-%d\n\"[\" is present, but it is not closed with \"]\"\n",
                peekToken(list)->pos);
                exit(1);
            }
        }
    }
    nextToken(list);
    
    newNode->as.literal.lit = lit;

    return newNode;
}

// コンマ
Node *comma(TokenList *list, Arena *arena) {
    Node *left = application(list, arena);

    if (peekToken(list)->type != TOKEN_COMMA) {
        return left;
    }

    nextToken(list); // ,

    Node *right = comma(list, arena);

    Node *node = arenaAlloc(arena, sizeof(Node));
    node->type = NODE_COMMA;
    node->pos = peekToken(list)->pos;
    node->as.comma.left = left;
    node->as.comma.right = right;

    return node;
}

// 識別子
Node *identifier(TokenList *list, Arena *arena) {
    Node *newNode = arenaAlloc(arena, sizeof(Node));

    if (peekToken(list)->type != TOKEN_IDENT) {
        printf("IdentifierError: Line-%d\nThe part that was supposed to be treated as a identifier was not a identifier\n",
                peekToken(list)->pos);
        exit(1);
    }

    newNode->type = NODE_IDENT;
    newNode->pos = peekToken(list)->pos;
    newNode->as.ident.ident = peekToken(list)->value;

    nextToken(list);

    return newNode;
}

// 後で実装しといてくれめんす
// ノードを表示
static void printIndent(int depth) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
}

void printNode(Node *node, int depth) {
    if (node == NULL) {
        printIndent(depth);
        printf("(null)\n");
        return;
    }

    printIndent(depth);

    switch (node->type) {

    case NODE_PROGRAM:
        printf("PROGRAM\n");
        printNode(node->as.prog.left, depth + 1);
        printNode(node->as.prog.right, depth + 1);
        break;

    case NODE_APP:
        printf("APP\n");
        printNode(node->as.app.func, depth + 1);
        printNode(node->as.app.arg, depth + 1);
        break;

    case NODE_COMMA:
        printf("COMMA\n");

        printNode(
            node->as.comma.left,
            depth + 1
        );

        printNode(
            node->as.comma.right,
            depth + 1
        );

        break;

    case NODE_LITERAL:
        switch (node->as.literal.lit.type) {
        case LITERAL_NUMBER:
            printf("NUMBER %g\n", node->as.literal.lit.numberLiteral);
            break;

        case LITERAL_STRING:
            printf("STRING %s\n", node->as.literal.lit.stringLiteral);
            break;

        case LITERAL_BOOL:
            printf("BOOL %s\n",
                   node->as.literal.lit.boolLiteral ? "true" : "false");
            break;

        case LITERAL_LIST:
            printf("LIST\n");

            printNode(
                node->as.literal.lit.listLiteral,
                depth + 1
            );

            break;
        }
        break;

    case NODE_IDENT:
        printf("IDENT %s\n", node->as.ident.ident);
        break;

    case NODE_QUOTE:
        printf("QUOTE\n");
        printNode(node->as.quote.node, depth + 1);
        break;

    default:
        printf("UNKNOWN NODE\n");
        break;
    }
}