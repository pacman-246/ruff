#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>
#include "../tokenizer/tokenizer.h"
#include "../arena/arena.h"

// ノードの種類
typedef enum {
    NODE_PROGRAM, // <program> ::= <statement>*
                  // <statement> ::= <expr> ";"
                  // <expr> ::= <application>
    NODE_APP,     // <application> ::= <atom>
                  //     | <application> "->" <atom>
                  // <atom> ::= <literal>
                  //     | <ident>
                  //     | "(" <expr> ")"
    NODE_LITERAL, // <literal> ::= <int>
                  //     | <float>
                  //     | <string>
                  //     | <bool>
    NODE_IDENT,   //<ident> ::= <letter> (<letter> | "_")*
} NodeType;

// リテラルの種類
typedef enum {
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_BOOL,
} LiteralType;

// リテラル
typedef struct Literal Literal;

struct Literal {
    LiteralType type;
    union {
        int i;
        double d;
        char *s;
        bool b;
    };
};

// ノード
typedef struct Node Node;

struct Node {
    NodeType type;

    union {
        // PROGRAMの場合
        struct {
            Node *left;
            Node *right;
        } prog;

        // APPの場合
        struct {
            Node *func;
            Node *arg;
        } app;

        // LITERALの場合
        struct {
            Literal lit;
        } literal;

        // IDENTの場合
        struct {
            char *ident;
        } ident;

    } as;
};

Node *parser(TokenList *list, Arena *arena);
Node *program(TokenList *list, Arena *arena);
Node *application(TokenList *list, Arena *arena);
Node *atom(TokenList *list, Arena *arena);
Node *literal(TokenList *list, Arena *arena);
Node *identifier(TokenList *list, Arena *arena);
static void printIndent(int depth);
void printNode(Node *node, int depth);

#endif
