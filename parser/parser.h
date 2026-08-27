#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>
#include "../tokenizer/tokenizer.h"
#include "../arena/arena.h"

// ノードの種類
typedef enum {
    NODE_PROGRAM,  // <program> ::= <statement>*
                   // <statement> ::= <application> ";"
    NODE_APP,      // <application> ::= <atom>
                   //     | <application> "->" <atom>
                   // <atom> ::= <literal>
                   //     | <ident>
                   //     | <quote>
                   //     | "(" <application> ")"
    NODE_LITERAL,  // <literal> ::= <int>
                   //     | <float>
                   //     | <string>
                   //     | <bool>
                   //     | <list>
    NODE_LIST,     // <list> ::= "[" <comma>? "]"
    NODE_COMMA,    // <comma> ::= <application> ("," <application>)*
    NODE_IDENT,    // <ident> ::= <letter> (<letter> | "_")*
    NODE_QUOTE,    // <quote> ::= "$" <atom>
} NodeType;

// リテラルの種類
typedef enum {
    LITERAL_NUMBER,
    LITERAL_STRING,
    LITERAL_BOOL,
    LITERAL_LIST
} LiteralType;

// リテラル
typedef struct Literal Literal;

typedef struct Node Node;

struct Literal {
    LiteralType type;
    union {
        double numberLiteral;
        char *stringLiteral;
        bool boolLiteral;
        Node *listLiteral;
    };
};

// ノード
struct Node {
    NodeType type;
    int pos;

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

        // COMMAの場合
        struct { 
            Node *right;
            Node *left;
        } comma;

        // IDENTの場合
        struct {
            char *ident;
        } ident;

        // QUOTEの場合
        struct {
            Node *node;
        } quote;

    } as;
};

Node *parser(TokenList *list, Arena *arena);
Node *program(TokenList *list, Arena *arena);
Node *application(TokenList *list, Arena *arena);
Node *atom(TokenList *list, Arena *arena);
Node *literal(TokenList *list, Arena *arena);
Node *comma(TokenList *list, Arena *arena);
Node *identifier(TokenList *list, Arena *arena);
static void printIndent(int depth);
void printNode(Node *node, int depth);

#endif