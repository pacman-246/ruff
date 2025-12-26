#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

ASTNode parser(Arena *arena, TokenList *list) {
    return *program(arena, list);
}

// 複数文
ASTNode *program(Arena* arena, TokenList *list) {
    ASTNode *node = comparison(arena, list);

    while (peekToken(list)->type == TOKEN_NEWLINE) {
        Token *t = nextToken(list);
        if (t->type == TOKEN_EOF) {
            break;
        }

        ASTNode *right = comparison(arena, list);

        ASTNode *newNode_ = newNode(arena, NODE_PROGRAM);
        newNode_->as.program.left = node;
        newNode_->as.program.right = right;

        node = newNode_;
    }

    return node;
}

// 比較演算子
ASTNode *comparison(Arena *arena, TokenList *list) {
    ASTNode *node = additive(arena, list);

    while (peekToken(list)->type == TOKEN_LT
        || peekToken(list)->type == TOKEN_LE
        || peekToken(list)->type == TOKEN_GT
        || peekToken(list)->type == TOKEN_GE
        || peekToken(list)->type == TOKEN_EQ
        || peekToken(list)->type == TOKEN_NE) {

        TokenType op = peekToken(list)->type;
        nextToken(list);

        ASTNode *right = additive(arena, list);

        ASTNode *newNode_ = newNode(arena, NODE_BINARY);
        newNode_->as.binary.left  = node;
        newNode_->as.binary.right = right;
        newNode_->as.binary.op    = op;

        node = newNode_;
    }

    return node;
}

// +と-
ASTNode *additive(Arena *arena, TokenList *list) {
    ASTNode *node = multiplicative(arena, list);

    while (peekToken(list)->type == TOKEN_PLUS
        || peekToken(list)->type == TOKEN_MINUS) {

        TokenType op = peekToken(list)->type;
        nextToken(list);

        ASTNode *right = multiplicative(arena, list);

        ASTNode *newNode_ = newNode(arena, NODE_BINARY);
        newNode_->as.binary.left = node;
        newNode_->as.binary.right = right;
        newNode_->as.binary.op = op;

        node = newNode_;
    }

    return node;
}

// *と/と%
ASTNode *multiplicative(Arena *arena, TokenList *list) {
    ASTNode *node = postfix(arena, list);

    while (peekToken(list)->type == TOKEN_STAR
        || peekToken(list)->type == TOKEN_SLASH
        || peekToken(list)->type == TOKEN_PERCENT) {

        TokenType op = peekToken(list)->type;
        nextToken(list);

        ASTNode *right = postfix(arena, list);

        ASTNode *newNode_ = newNode(arena, NODE_BINARY);
        newNode_->as.binary.left  = node;
        newNode_->as.binary.right = right;
        newNode_->as.binary.op    = op;

        node = newNode_;
    }

    return node;
}

// .と関数呼び出し
ASTNode *postfix(Arena *arena, TokenList *list) {
    ASTNode *node = primary(arena, list);

    // trueのほうがわかりやすいが無駄なincludeが増えるため1
    while (1) {
        if (peekToken(list)->type == TOKEN_DOT) {
            nextToken(list);

            ASTNode *right = ident(arena, list);

            ASTNode *newNode_ = newNode(arena, NODE_BINARY);
            newNode_->as.binary.left  = node;
            newNode_->as.binary.right = right;
            newNode_->as.binary.op    = TOKEN_DOT;

            node = newNode_;

        } else if (peekToken(list)->type == TOKEN_LPAREN) {
            nextToken(list); // (

            ASTNode *args = NULL;
            if (peekToken(list)->type == TOKEN_RPAREN) {
                // 引数なり
                args = newNode(arena, NODE_EMPTY);
                
            } else {
                // 引数あり
                args = comma(arena, list);
            }

            if (peekToken(list)->type != TOKEN_RPAREN) {
                printf("the closing parenthesis is missing\n");
                exit(1);
            }
            nextToken(list); // )

            ASTNode *call = newNode(arena, NODE_FUNCCALL);
            call->as.funcCall.name = node;
            call->as.funcCall.args = args;

            node = call;
        } else {
            break;
        }
    }

    return node;
}

// ,
ASTNode *comma(Arena *arena, TokenList *list) {
    ASTNode *node = comparison(arena, list);

    while (peekToken(list)->type == TOKEN_COMMA) {
        nextToken(list);

        ASTNode *right = comparison(arena, list);

        ASTNode *newNode_ = newNode(arena, NODE_COMMA);
        newNode_->as.comma.left  = node;
        newNode_->as.comma.right = right;

        node = newNode_;
    }

    return node;
}

// ()と識別子とリテラル
ASTNode *primary(Arena *arena, TokenList *list) {
    if (peekToken(list)->type == TOKEN_LPAREN) {
        nextToken(list);
        ASTNode *node = comparison(arena, list);

        if (peekToken(list)->type != TOKEN_RPAREN) {
            printf("the closing parenthesis is missing\n");
            exit(1);
        }
        nextToken(list);
        return node;
    }

    if (peekToken(list)->type == TOKEN_IDENT) {
        return ident(arena, list);
    }

    return literal(arena, list);
}

ASTNode *literal(Arena *arena, TokenList *list) {
    ASTNode *lit = newNode(arena, NODE_LITERAL);

    if (peekToken(list)->type == TOKEN_NUMBER) {
        lit->as.literal.kind = LIT_NUMBER;
        lit->as.literal.value.number =
            strtod(peekToken(list)->value, NULL);
    }
    else if (peekToken(list)->type == TOKEN_STRING) {
        lit->as.literal.kind = LIT_STRING;
        lit->as.literal.value.string = peekToken(list)->value;
    }
    else if (peekToken(list)->type == TOKEN_TRUE
        || peekToken(list)->type == TOKEN_FALSE) {
        lit->as.literal.kind = LIT_BOOL;
        lit->as.literal.value.boolean =
            peekToken(list)->type == TOKEN_TRUE;
    }

    nextToken(list);
    return lit;
}

ASTNode *ident(Arena *arena, TokenList *list) {
    ASTNode *id = newNode(arena, NODE_IDENT);
    id->as.ident.name = peekToken(list)->value;
    nextToken(list);
    return id;
}

ASTNode *newNode(Arena *arena, NodeType type) {
    ASTNode *node = arenaAlloc(arena, sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    return node;
}

char *nodeTypeName(NodeType type) {
    switch (type) {
        case NODE_EMPTY:          return "empty";
        case NODE_PROGRAM:        return "program";
        case NODE_IDENT:          return "ident";
        case NODE_LITERAL:        return "literal";
        case NODE_BINARY:         return "binary";
        case NODE_IF:             return "if";
        case NODE_WHILE:          return "while";
        case NODE_FOR:            return "for";
        case NODE_BREAK:          return "break";
        case NODE_CONTINUE:       return "continue";
        case NODE_PASS:           return "pass";
        case NODE_VARDECL:        return "varDecl";
        case NODE_ASSIGN:         return "assign";
        case NODE_FUNCDEF:        return "funcDef";
        case NODE_RETURN:         return "return";
        case NODE_FUNCCALL:       return "funcCall";
        case NODE_COMMA:          return "comma";
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
    printf("====ast====\n");
    _printAST(node, indent);
    printf("========\n");
}

void _printAST(ASTNode *node, int indent) {
    if (!node) {
        printIndent(indent);
        printf("(null)\n");
        return;
    }

    printIndent(indent);
    printf("%s", nodeTypeName(node->type));

    switch (node->type) {

        case NODE_EMPTY: {
            // emptyと表示するだけで良いので改行のみ
            printf("\n");
            break;
        }

        case NODE_PROGRAM: {
            printf("\n");

            if (node->as.program.left) {
                _printAST(node->as.program.left, indent + 1);
            }

            printf("\n");

            if (node->as.program.right) {
                _printAST(node->as.program.right, indent + 1);
            }
            break;
        }

        case NODE_IDENT: {
            printf(" name=%s\n", node->as.ident.name);
            break;
        }

        case NODE_LITERAL: {
            printLiteral(node);
            printf("\n");
            break;
        }

        case NODE_BINARY: {
            printf(" op=%s\n", tokenTypeToString(node->as.binary.op));
            _printAST(node->as.binary.left, indent + 1);
            _printAST(node->as.binary.right, indent + 1);
            break;
        }

        case NODE_IF: {
            printf("\n");

            printIndent(indent + 1);
            printf("Condition:\n");
            _printAST(node->as.ifStmt.condition, indent + 2);

            printIndent(indent + 1);
            printf("Then:\n");
            _printAST(node->as.ifStmt.thenBranch, indent + 2);

            if (node->as.ifStmt.elseBranch) {
                printIndent(indent + 1);
                printf("Else:\n");
                _printAST(node->as.ifStmt.elseBranch, indent + 2);
            }
            break;
        }

        case NODE_WHILE: {
            printf("\n");
            printIndent(indent + 1);
            printf("Condition:\n");
            _printAST(node->as.whileStmt.condition, indent + 2);

            printIndent(indent + 1);
            printf("Body:\n");
            _printAST(node->as.whileStmt.body, indent + 2);
            break;
        }

        case NODE_FOR: {
            printf(" var=%s\n", node->as.forStmt.varName);

            printIndent(indent + 1);
            printf("Iterable:\n");
            _printAST(node->as.forStmt.iterable, indent + 2);

            printIndent(indent + 1);
            printf("Body:\n");
            _printAST(node->as.forStmt.body, indent + 2);
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
                _printAST(node->as.varDecl.type, indent + 2);
            }

            if (node->as.varDecl.value) {
                printIndent(indent + 1);
                printf("Init:\n");
                _printAST(node->as.varDecl.value, indent + 2);
            }
            break;
        }

        case NODE_ASSIGN: {
            printf(" name=%s\n", node->as.assign.name);
            _printAST(node->as.assign.value, indent + 1);
            break;
        }

        case NODE_FUNCDEF: {
            printf(" name=%s params=%d\n",
                node->as.funcDef.name,
                node->as.funcDef.paramCount
            );

            for (int i = 0; i < node->as.funcDef.paramCount; i++) {
                _printAST(node->as.funcDef.params[i], indent + 1);
            }

            printIndent(indent + 1);
            printf("Body:\n");
            _printAST(node->as.funcDef.body, indent + 2);
            break;
        }

        case NODE_RETURN: {
            printf("\n");

            if (node->as.returnStmt.value) {
                printIndent(indent + 1);
                printf("Value:\n");
                _printAST(node->as.returnStmt.value, indent + 2);
            }

            break;
        }

        case NODE_FUNCCALL: {
            printf("\n");

            printIndent(indent + 1);
            printf("Name:\n");
            _printAST(node->as.funcCall.name, indent + 2);

            if (node->as.funcCall.args) {
                printIndent(indent + 1);
                printf("Args:\n");
                _printAST(node->as.funcCall.args, indent + 2);
            }
            break;
        }

        case NODE_COMMA: {
            printf("\n");

            if (node->as.comma.left) {
                _printAST(node->as.comma.left, indent + 1);
            }

            printf("\n");

            if (node->as.comma.right) {
                _printAST(node->as.comma.right, indent + 1);
            }
            break;
        }

        case NODE_BREAK:
        case NODE_CONTINUE:
        case NODE_PASS:
            printf("\n");
            break;

        default:
            printf(" (unhandled)\n");
    }
}

void arenaInit(Arena *arena, size_t capacity) {
    arena->memory = malloc(capacity);
    arena->capacity = capacity;
    arena->used = 0;
}

void arenaFree(Arena *arena) {
    free(arena->memory);
    arena->memory = NULL;
    arena->capacity = arena->used = 0;
}

void *arenaAlloc(Arena *arena, size_t size) {
    size = (size + 7) & ~7; // 8byte alignment

    if (arena->used + size > arena->capacity) {
        return NULL; // 足りない（本番では assert）
    }

    void *ptr = arena->memory + arena->used;
    arena->used += size;
    return ptr;
}
