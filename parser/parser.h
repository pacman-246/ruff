#ifndef _PARSER_H_
#define _PARSER_H_

#include <string.h>
#include "../tokenizer/tokenizer.h"

typedef enum {
    DECL_VAR,
    DECL_LET
} DeclKind;

typedef enum {
    LIT_NUMBER,
    LIT_STRING,
    LIT_BOOL
} LiteralKind;

typedef enum {
    // 引数のない関数など
    NODE_EMPTY,

    NODE_PROGRAM,
    NODE_LITERAL,
    NODE_IDENT,
    NODE_BINARY,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_PASS,
    NODE_VARDECL,
    NODE_ASSIGN,
    NODE_FUNCDEF,
    NODE_RETURN,
    NODE_FUNCCALL,
    NODE_COMMA,

} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;

    union {
        struct {
            ASTNode *left;
            ASTNode *right;
        } program;

        // リテラル
        struct {
            LiteralKind kind;
            union {
                double number;
                char *string;
                int boolean;
            } value;
        } literal;

        // 識別子
        struct {
            char *name;
        } ident;
        
        // 二項演算
        struct {
            ASTNode *left;
            ASTNode *right;
            TokenType op;
        } binary;

        // if文
        struct {
            ASTNode *condition;
            ASTNode *thenBranch;
            ASTNode *elseBranch; // elseまたはelif
        } ifStmt;

        // while文
        struct {
            ASTNode *condition;
            ASTNode *body;
        } whileStmt;

        // for文
        struct {
            char *varName;
            ASTNode *iterable;
            ASTNode *body;
        } forStmt;

        // break
        struct {
            // 何も持たない
        } breakStmt;

        // continue
        struct {
            // 何も持たない
        } continueStmt;

        // pass
        struct {
            // 何も持たない
        } passStmt;

        // 宣言
        struct {
            DeclKind kind;
            char *name;
            ASTNode *type;
            ASTNode *value;
        } varDecl;

        // 代入
        struct {
            char *name;
            ASTNode *value;
        } assign;
        
        // 関数定義
        struct {
            char *name;
            ASTNode **params;
            int paramCount;
            ASTNode *body;
        } funcDef;

        // return文
        struct {
            ASTNode *value;
        } returnStmt;

        // 関数呼び出し
        struct {
            ASTNode *name; // fmt.printlnのような形の関数だからあえてASTNode
            ASTNode *args;
        } funcCall;

        // コンマ
        struct {
            ASTNode *left;
            ASTNode *right;
        } comma;

    } as;
};

// ASTNodeのfreeを軽量化するためのArena
typedef struct {
    unsigned char *memory;
    size_t capacity;
    size_t used;
} Arena;

ASTNode parser(Arena *arena, TokenList *list);
ASTNode *program(Arena *arena, TokenList *list);
ASTNode *comparison(Arena *arena, TokenList *list);
ASTNode *additive(Arena *arena, TokenList *list);
ASTNode *multiplicative(Arena *arena, TokenList *list);
ASTNode *postfix(Arena *arena, TokenList *list);
ASTNode *comma(Arena *arena, TokenList *list);
ASTNode *primary(Arena *arena, TokenList *list);
ASTNode *literal(Arena *arena, TokenList *list);
ASTNode *ident(Arena *arena, TokenList *list);
ASTNode *newNode(Arena *arena, NodeType type);
char *nodeTypeName(NodeType type);
static void printLiteral(ASTNode *node);
static void printIndent(int indent);
void printAST(ASTNode *node, int indent);
void _printAST(ASTNode *node, int indent);
void arenaInit(Arena *arena, size_t capacity);
void arenaFree(Arena *arena);
void *arenaAlloc(Arena *arena, size_t size);

#endif
