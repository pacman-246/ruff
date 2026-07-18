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
        newNode->as.app.func = node;
        newNode->as.app.arg = right;

        node = newNode;
    }

    return node;
}

// <atom> ::= <literal>
//     | <identifier>
//     | "(" <expr> ")"
Node *atom(TokenList *list, Arena *arena) {
    TokenType tt = peekToken(list)->type;
    switch (tt) {
        // <literal>
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_BOOL:
            return literal(list, arena);

        // <identifier>
        case TOKEN_IDENT:
            return identifier(list, arena);

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
    }
}

// リテラル
Node *literal(TokenList *list, Arena *arena) {
    Node *newNode = arenaAlloc(arena, sizeof(Node));

    TokenType tt = peekToken(list)->type;
    if (tt != TOKEN_INT
        && tt != TOKEN_FLOAT
        && tt != TOKEN_STRING
        && tt != TOKEN_BOOL) {
        printf("LiteralError: Line-%d\nThe part that was supposed to be treated as a literal was not a literal\n",
                peekToken(list)->pos);
        exit(1);
    }

    newNode->type = NODE_LITERAL;
    struct Literal lit;

    char *value = peekToken(list)->value;

    if (tt == TOKEN_INT) {
        lit.type = LITERAL_INT;
        lit.i = atoi(value);

    } else if (tt == TOKEN_FLOAT) {
        lit.type = LITERAL_FLOAT;
        lit.d = atof(value);

    } else if (tt == TOKEN_STRING) {
        lit.type = LITERAL_STRING;
        lit.s = value;

    } else if (tt == TOKEN_BOOL) {
        lit.type = LITERAL_BOOL;
        if (strcmp(value, "true") == 0) {
            lit.b = true;
        } else if (strcmp(value, "false") == 0) {
            lit.b = false;
        }
    }

    newNode->as.literal.lit = lit;

    nextToken(list);

    return newNode;
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
    newNode->as.ident.ident = peekToken(list)->value;

    nextToken(list);

    return newNode;
}

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

    case NODE_LITERAL:
        switch (node->as.literal.lit.type) {
        case LITERAL_INT:
            printf("INT %d\n", node->as.literal.lit.i);
            break;

        case LITERAL_FLOAT:
            printf("FLOAT %f\n", node->as.literal.lit.d);
            break;

        case LITERAL_STRING:
            printf("STRING \"%s\"\n", node->as.literal.lit.s);
            break;

        case LITERAL_BOOL:
            printf("BOOL %s\n",
                   node->as.literal.lit.b ? "true" : "false");
            break;
        }
        break;

    case NODE_IDENT:
        printf("IDENT %s\n", node->as.ident.ident);
        break;

    default:
        printf("UNKNOWN NODE\n");
        break;
    }
}