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
    NODE_PROGRAM,

    NODE_LITERAL,
    NODE_VARIABLE,
    NODE_BINARY,
    NODE_UNARY,
    NODE_IFSTMT,
    NODE_WHILESTMT,
    NODE_FORSTMT,
    NODE_BREAKSTMT,
    NODE_CONTINUESTMT,
    NODE_PASSSTMT,
    NODE_VARDECL,
    NODE_ASSIGN,
    NODE_FUNCDEF,
    NODE_FUNCCALL

} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;

    union {
        struct {
            ASTNode **statements;
            int count;
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

        // 変数名
        struct {
            char *name;
        } variable;
        
        // 二項演算
        struct {
            ASTNode *left;
            ASTNode *right;
            TokenType op;
        } binary;

        // 単項演算（++、--など）
        struct {
            ASTNode *expr;
            TokenType op;
        } unary;

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

        // 関数呼び出し
        struct {
            char *name;
            ASTNode **args;
            int argCount;
        } funcCall;

    } as;
};

ASTNode parser(TokenList *list);
ASTNode expr(TokenList *list);
ASTNode term(TokenList *list);
ASTNode factor(TokenList *list);
ASTNode literal(TokenList *list);
ASTNode *newNode(NodeType type);
char *nodeTypeName(NodeType type);
static void printLiteral(ASTNode *node);
static void printIndent(int indent);
void printAST(ASTNode *node, int indent);

#endif