#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

ASTNode parser(TokenList *list) {
    return expr(list);
}

// +と-
ASTNode expr(TokenList *list) {
    ASTNode node = term(list);

    while (peekToken(list)->type == TOKEN_PLUS
        || peekToken(list)->type == TOKEN_MINUS) {
        char *op = peekToken(list)->value;

        ASTNode *_newNode = newNode(NODE_BINARY);

        // left
        ASTNode *left = malloc(sizeof(ASTNode));
        *left = node;
        _newNode->as.binary.left = left;

        // op
        if (strcmp(op, "+") == 0) {
            _newNode->as.binary.op = TOKEN_PLUS;
        } else if (strcmp(op, "-") == 0) {
            _newNode->as.binary.op = TOKEN_MINUS;
        }
        // 演算子を消費する
        nextToken(list);

        // right
        ASTNode *right = malloc(sizeof(ASTNode));
        *right = term(list);
        _newNode->as.binary.right = right;

        node = *_newNode;
    }

    return node;
}

// *と/
ASTNode term(TokenList *list) {
    ASTNode node = factor(list);

    while (peekToken(list)->type == TOKEN_STAR
        || peekToken(list)->type == TOKEN_SLASH) {
        char *op = peekToken(list)->value;

        ASTNode *_newNode = newNode(NODE_BINARY);

        // left
        ASTNode *left = malloc(sizeof(ASTNode));
        *left = node;
        _newNode->as.binary.left = left;

        // op
        if (strcmp(op, "*") == 0) {
            _newNode->as.binary.op = TOKEN_STAR;
        } else if (strcmp(op, "/") == 0) {
            _newNode->as.binary.op = TOKEN_SLASH;
        }
        // 演算子を消費する
        ASTNode *right = malloc(sizeof(ASTNode));
        *right = factor(list);
        _newNode->as.binary.right = right;

        // right
        *_newNode->as.binary.right = factor(list);

        node = *_newNode;
    }

    return node;
}

// ()
ASTNode factor(TokenList *list) {
    if (peekToken(list)->type == TOKEN_LPAREN) { // (
        // (を消費
        nextToken(list);

        ASTNode node = expr(list);

        // この場合括弧が閉じられていないのでエラー
        if (peekToken(list)->type != TOKEN_RPAREN) { // )
            printf("the closing parenthesis is missing");
            exit(1);
        }
        // )を消費
        nextToken(list);

        return node;

    } else {
        return literal(list);
    }
}

ASTNode literal(TokenList *list) {
    ASTNode *lit = newNode(NODE_LITERAL);
    
    if (peekToken(list)->type == TOKEN_NUMBER) {
        lit->as.literal.kind = LIT_NUMBER;
        char *num = peekToken(list)->value;
        char *end;

        double x = strtod(num, &end);

        lit->as.literal.value.number = x;

    } else if (peekToken(list)->type == TOKEN_STRING) {
        lit->as.literal.kind = LIT_STRING;
        lit->as.literal.value.string = peekToken(list)->value;

    } else if (peekToken(list)->type == TOKEN_TRUE
            || peekToken(list)->type == TOKEN_FALSE) {
        lit->as.literal.kind = LIT_BOOL;
        // trueなら1、falseなら0
        lit->as.literal.value.boolean = peekToken(list)->type == TOKEN_TRUE ? 1 : 2;
    }

    // リテラルを消費
    nextToken(list);

    return *lit;
}

ASTNode *newNode(NodeType type) {
    ASTNode *n = malloc(sizeof(ASTNode));
    memset(n, 0, sizeof(ASTNode));
    n->type = type;
    return n;
}

char *nodeTypeName(NodeType type) {
    switch (type) {
        case NODE_PROGRAM:        return "program";
        case NODE_VARIABLE:       return "variable";
        case NODE_LITERAL:        return "literal";
        case NODE_BINARY:         return "binary";
        case NODE_UNARY:          return "unary";
        case NODE_IFSTMT:         return "if";
        case NODE_WHILESTMT:      return "while";
        case NODE_FORSTMT:        return "for";
        case NODE_BREAKSTMT:      return "break";
        case NODE_CONTINUESTMT:   return "continue";
        case NODE_PASSSTMT:       return "pass";
        case NODE_VARDECL:        return "varDecl";
        case NODE_ASSIGN:         return "assign";
        case NODE_FUNCDEF:        return "funcDef";
        case NODE_FUNCCALL:       return "funcCall";
        default:                  return "unknown";
    }
}

static void printLiteral(ASTNode *node) {
    switch (node->as.literal.kind) {
        case LIT_NUMBER:
            printf(" number=%f", node->as.literal.value.number);
            break;
        case LIT_STRING:
            printf(" string=\"%s\"", node->as.literal.value.string);
            break;
        case LIT_BOOL:
            printf(" bool=%s",
                node->as.literal.value.boolean ? "true" : "false");
            break;
    }
}

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");  // ← インデント1段 = 空白2つ
    }
}

void printAST(ASTNode *node, int indent) {
    if (!node) {
        printIndent(indent);
        printf("(null)\n");
        return;
    }

    printIndent(indent);
    printf("%s", nodeTypeName(node->type));

    switch (node->type) {

        case NODE_PROGRAM: {
            printf("\n");
            for (int i = 0; i < node->as.program.count; i++) {
                printAST(node->as.program.statements[i], indent + 1);
            }
            break;
        }

        case NODE_VARIABLE: {
            printf(" name=%s\n", node->as.variable.name);
            break;
        }

        case NODE_LITERAL: {
            printLiteral(node);
            printf("\n");
            break;
        }

        case NODE_BINARY: {
            printf(" op=%s\n", tokenTypeToString(node->as.binary.op));
            printAST(node->as.binary.left, indent + 1);
            printAST(node->as.binary.right, indent + 1);
            break;
        }

        case NODE_UNARY: {
            printf(" op=%d\n", node->as.unary.op);
            printAST(node->as.unary.expr, indent + 1);
            break;
        }

        case NODE_IFSTMT: {
            printf("\n");

            printIndent(indent + 1);
            printf("Condition:\n");
            printAST(node->as.ifStmt.condition, indent + 2);

            printIndent(indent + 1);
            printf("Then:\n");
            printAST(node->as.ifStmt.thenBranch, indent + 2);

            if (node->as.ifStmt.elseBranch) {
                printIndent(indent + 1);
                printf("Else:\n");
                printAST(node->as.ifStmt.elseBranch, indent + 2);
            }
            break;
        }

        case NODE_WHILESTMT: {
            printf("\n");
            printIndent(indent + 1);
            printf("Condition:\n");
            printAST(node->as.whileStmt.condition, indent + 2);

            printIndent(indent + 1);
            printf("Body:\n");
            printAST(node->as.whileStmt.body, indent + 2);
            break;
        }

        case NODE_FORSTMT: {
            printf(" var=%s\n", node->as.forStmt.varName);

            printIndent(indent + 1);
            printf("Iterable:\n");
            printAST(node->as.forStmt.iterable, indent + 2);

            printIndent(indent + 1);
            printf("Body:\n");
            printAST(node->as.forStmt.body, indent + 2);
            break;
        }

        case NODE_VARDECL: {
            printf(" name=%s kind=%s\n",
                node->as.varDecl.name,
                node->as.varDecl.kind == DECL_VAR ? "var" : "let"
            );

            if (node->as.varDecl.type) {
                printIndent(indent + 1);
                printf("Type:\n");
                printAST(node->as.varDecl.type, indent + 2);
            }

            if (node->as.varDecl.value) {
                printIndent(indent + 1);
                printf("Init:\n");
                printAST(node->as.varDecl.value, indent + 2);
            }
            break;
        }

        case NODE_ASSIGN: {
            printf(" name=%s\n", node->as.assign.name);
            printAST(node->as.assign.value, indent + 1);
            break;
        }

        case NODE_FUNCDEF: {
            printf(" name=%s params=%d\n",
                node->as.funcDef.name,
                node->as.funcDef.paramCount
            );

            for (int i = 0; i < node->as.funcDef.paramCount; i++) {
                printAST(node->as.funcDef.params[i], indent + 1);
            }

            printIndent(indent + 1);
            printf("Body:\n");
            printAST(node->as.funcDef.body, indent + 2);
            break;
        }

        case NODE_FUNCCALL: {
            printf(" name=%s args=%d\n",
                node->as.funcCall.name,
                node->as.funcCall.argCount
            );

            for (int i = 0; i < node->as.funcCall.argCount; i++) {
                printAST(node->as.funcCall.args[i], indent + 1);
            }
            break;
        }

        case NODE_BREAKSTMT:
        case NODE_CONTINUESTMT:
        case NODE_PASSSTMT:
            printf("\n");
            break;

        default:
            printf(" (unhandled)\n");
    }
}
