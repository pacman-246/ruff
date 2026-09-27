#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../parser/parser.h"
#include "evaluator.h"

Value evaluator(Node *node, Environment *env, Arena *arena) {
    switch (node->type) {
        case NODE_PROGRAM: // 複数文
            evaluator(node->as.prog.left, env, arena);
            evaluator(node->as.prog.right, env, arena);

            return newUnitValue(arena);

        case NODE_APP: // 関数適用
            Value func = evaluator(node->as.app.func, env, arena);
            Value arg = evaluator(node->as.app.arg, env, arena);
            return apply(func, arg, env, node->pos, arena);

        case NODE_LITERAL: // リテラル
            return literalToValue(node, env, arena);

        case NODE_COMMA: // リストの中身のコンマの処理
            List *list = arenaAlloc(arena, sizeof(List));

            // 左側が現在の要素
            list->head =
                evaluator(node->as.comma.left, env, arena);

            if (node->as.comma.right->type == NODE_COMMA) {

                // 右側にもCOMMAが続いている
                Value right =
                    evaluator(node->as.comma.right, env, arena);

                list->tail = right.listValue;

            } else {

                // 最後の要素
                List *last = arenaAlloc(arena, sizeof(List));

                last->head =
                    evaluator(node->as.comma.right, env, arena);

                last->tail = NULL;

                list->tail = last;
            }

            return newListValue(list, arena);

        case NODE_IDENT: // 識別子
            return getValueFromIdent(node, env);

        case NODE_QUOTE: // quote
            return newQuoteValue(node->as.quote.node, arena);

        default:
            return newUnitValue(arena);
    }
}

Value apply(Value func, Value arg, Environment *env, int pos, Arena *arena) {
    if (func.type == VALUE_BUILTINFUNCTION) {
        return func.builtinFnValue(
            newBuiltinFnCtx(
                arg,
                env,
                pos,
                arena
            )
        );
        
    } else if (func.type == VALUE_FUNCTION) {
        Function *f = &func.functionValue;

        // 新しい環境を用意
        Environment *newEnv = newEnvironment(f->parentEnv, arena);

        // 引数を入れる
        define(newEnv, f->param, arg, arena);

        // 実行
        return evaluator(f->body, newEnv, arena);
    
    } else if (func.type == VALUE_CLANGFUNCTION) {
        return func.clangFnValue.clangFnValue(
            *func.clangFnValue.setValue,
            newBuiltinFnCtx(
                arg,
                env,
                pos,
                arena
            )
        );

    } else { // 関数以外のvalueで関数適用しているのでエラー
        printf("TypeError: Line-%d\nA non-function is being applied as a function\n",
        pos);
        exit(1);
    }
}

// リテラルをValueに変換
Value literalToValue(Node *node, Environment *env, Arena *arena) {
    Value *v = arenaAlloc(arena, sizeof(Value));
    
    Literal l = node->as.literal.lit;
    LiteralType lt = l.type;
    switch (lt) {
        case LITERAL_NUMBER:
            v->type = VALUE_NUMBER;
            v->numberValue = l.numberLiteral;
            break;

        case LITERAL_STRING:
            v->type = VALUE_STRING;
            v->stringValue = l.stringLiteral;
            break;
        
        case LITERAL_BOOL:
            v->type = VALUE_BOOL;
            v->boolValue = l.boolLiteral;
            break;

        case LITERAL_LIST:
            v->type = VALUE_LIST;

            if (l.listLiteral == NULL) {
                v->listValue = NULL;
                break;
            }

            Value list = evaluator(l.listLiteral, env, arena);
            if (list.type == VALUE_LIST) {
                v->listValue = list.listValue;
            } else if (!list.type) {
                List *newList = arenaAlloc(arena, sizeof(List));
                newList->head = list;
                newList->tail = NULL;
                v->listValue = newList;
            }
            break;

        default:
            break;
    }

    return *v;
}

// identを定数名として捉えその定数のValueを取得する
Value getValueFromIdent(Node *node, Environment *env) {
    char *ident = node->as.ident.ident;

    bool thereIsVal = false;
    Value val;
    for (int i = 0; i < env->count; i++) {
        if (strcmp((env->constants + i)->name, ident) == 0) {
            val = (env->constants + i)->value;
            thereIsVal = true;
            break;
        }
    }
    if (!thereIsVal) {
        if (env->parent == NULL) {
            // 親がNULLということはグローバル環境ということなので
            // その環境でなかったということは本当にないのでエラー
            printf("NameError: Line-%d\nName %s is not define\n",
                node->pos, ident);
            exit(1);
        } else {
            return getValueFromIdent(node, env->parent);
        }
    }

    return val;
}

// environment関係

// environmentを初期化
Environment *newEnvironment(Environment *parent, Arena *arena) {
    Environment *env = arenaAlloc(arena, sizeof(Environment));

    env->count = 0;
    env->capacity = 8;
    env->parent = parent;
    env->constants = arenaAlloc(arena, sizeof(Constant) * env->capacity);

    return env;
}

// environmentに定数を定義（define）をする
void define(Environment *env, char *name, Value value, Arena *arena) {
    if (env->count >= env->capacity) {
        size_t newCapacity = env->capacity * 2;

        Constant *newConstants =
            arenaAlloc(arena, sizeof(Constant) * newCapacity);

        memcpy(newConstants,
               env->constants,
               sizeof(Constant) * env->count);

        env->constants = newConstants;
        env->capacity = newCapacity;
    }

    env->constants[env->count].name = name;
    env->constants[env->count].value = value;
    env->count++;
}

// ビルトイン関数を定義

BuiltinFnCtx *newBuiltinFnCtx(Value arg, Environment *env, int pos, Arena *arena) {
    BuiltinFnCtx *ctx = arenaAlloc(arena, sizeof(BuiltinFnCtx));
    ctx->arg = arg;
    ctx->env = env;
    ctx->pos = pos;
    ctx->arena = arena;
    return ctx;
}

ClangFunction newClangFunction(
    Value (*clangFnValue)(Value setValue, BuiltinFnCtx *ctx),
    Value setValue,
    Arena *arena) {

    ClangFunction *cf = arenaAlloc(arena, sizeof(ClangFunction));

    Value *saved = arenaAlloc(arena, sizeof(Value));
    *saved = setValue;

    cf->clangFnValue = clangFnValue;
    cf->setValue = saved;

    return *cf;
}

Value builtinPrintln(BuiltinFnCtx *ctx) {
    printf(
        "%s\n",
        valueToString(ctx->arg, ctx->arena)
    );

    return newUnitValue(ctx->arena);
}

// 足し算
Value builtinAdd(BuiltinFnCtx *ctx) {
    // 引数の型チェック
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe first level \"add\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(addSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value addSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe second level \"add\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newNumberValue(
        setValue.numberValue + ctx->arg.numberValue,
        ctx->arena
    );
}

// 引き算
Value builtinSub(BuiltinFnCtx *ctx) {
    // 引数の型チェック
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe first level \"sub\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(subSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value subSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe second level \"sub\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newNumberValue(
        setValue.numberValue - ctx->arg.numberValue,
        ctx->arena
    );
}

// 掛け算
Value builtinMul(BuiltinFnCtx *ctx) {
    // 引数の型チェック
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe first level \"mul\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(mulSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value mulSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe second level \"mul\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newNumberValue(
        setValue.numberValue * ctx->arg.numberValue,
        ctx->arena
    );
}

// 割り算
Value builtinDiv(BuiltinFnCtx *ctx) {
    // 引数の型チェック
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe first level \"div\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(divSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value divSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe second level \"div\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newNumberValue(
        setValue.numberValue / ctx->arg.numberValue,
        ctx->arena
    );
}

// あまり
Value builtinMod(BuiltinFnCtx *ctx) {
    // 引数の型チェック
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe first level \"mod\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(modSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value modSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_NUMBER) {
        printf("TypeError: Line-%d\nThe second level \"mod\" argument only accepts the \"Number\" type\n",
        ctx->pos);
        exit(1);
    }

    return newNumberValue(
        fmod(setValue.numberValue, ctx->arg.numberValue),
        ctx->arena
    );
}

Value builtinIf(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_BOOL) {
        printf("TypeError: Line-%d\nThe first level \"if\" argument only accepts the \"Bool\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(ifSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value ifSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (setValue.boolValue) {
        return newClangFunctionValue(
            newClangFunction(ifThird, ctx->arg, ctx->arena),
            ctx->arena
        );
    } else {
        return newClangFunctionValue(
            newClangFunction(ifFourth, ctx->arg, ctx->arena),
            ctx->arena
        );
    }
}

// trueのときはレベル2の関数の引数を実行し、
// falseのときはレベル3の関数の引数を実行する
Value ifThird(Value setValue, BuiltinFnCtx *ctx) {
    return setValue;
}

Value ifFourth(Value setValue, BuiltinFnCtx *ctx) {
    return ctx->arg;
}

Value builtinEqual(BuiltinFnCtx *ctx) {
    return newClangFunctionValue(
        newClangFunction(equalSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value equalSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (setValue.type != ctx->arg.type) {
        printf("TypeError: Line-%d\nAll arguments for \"equal\" must be of the same type\n",
        ctx->pos);
        exit(1);
    }

    Value *result = arenaAlloc(ctx->arena, sizeof(Value));
    result->type = VALUE_BOOL;

    // 型は同じなので片方だけ調べればよし
    switch (setValue.type) {
        case VALUE_NUMBER:
            bool r = (setValue.numberValue == ctx->arg.numberValue);
            result->boolValue = r;
            break;

        case VALUE_STRING:
            bool r_ = (strcmp(setValue.stringValue, ctx->arg.stringValue) == 0);
            result->boolValue = r_;
            break;

        case VALUE_BOOL:
            bool r__ = (setValue.boolValue == ctx->arg.boolValue);
            result->boolValue = r__;
            break;

        case VALUE_UNIT:
            result->boolValue = true;
            break;

        case VALUE_LIST:
            List *x = setValue.listValue;
            List *y = ctx->arg.listValue;

            while (x != NULL && y != NULL) {
                if (!equalSecond(
                        x->head,
                        newBuiltinFnCtx(
                            y->head,
                            ctx->env,
                            ctx->pos,
                            ctx->arena
                        )    
                    ).boolValue) {
                    result->boolValue = false;
                    break;
                }

                x = x->tail;
                y = y->tail;
            }

            result->boolValue = x == NULL && y == NULL;
            break;

        default:
            result->boolValue = false;
            break;
    }

    return *result;
}

Value builtinLet(BuiltinFnCtx *ctx) {
    // 引数の型チェック
    if (ctx->arg.type != VALUE_QUOTE
        || ctx->arg.quoteValue->type != NODE_IDENT) {
        printf("TypeError: Line-%d\nThe first level \"let\" argument only accepts the \"Quote\" type(ident)\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(letSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value letSecond(Value setValue, BuiltinFnCtx *ctx) {
    char *constName = setValue.quoteValue->as.ident.ident;

    define(ctx->env, constName, ctx->arg, ctx->arena);

    return newUnitValue(ctx->arena);
}

Value builtinFn(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_QUOTE
        || ctx->arg.quoteValue->type != NODE_IDENT) {
        printf("TypeError: Line-%d\nThe first level \"fn\" argument only accepts the \"Quote\" type(ident)\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(fnSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value fnSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_QUOTE) {
        printf("TypeError: Line-%d\nThe second level \"fn\" argument only accepts the \"Quote\" type\n",
        ctx->pos);
        exit(1);
    }

    char *argName = setValue.quoteValue->as.ident.ident;

    Function *f = arenaAlloc(ctx->arena, sizeof(Function));
    f->body = ctx->arg.quoteValue;
    f->param = argName;
    f->parentEnv = ctx->env;

    return newFunctionValue(*f, ctx->arena);
}

Value builtinRange(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_NUMBER
        || fmod(ctx->arg.numberValue, 1.0) != 0.0) {
        printf("TypeError: Line-%d\nThe first level \"range\" argument only accepts the \"Number\" type(integer)\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(rangeSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value rangeSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_NUMBER
        || fmod(ctx->arg.numberValue, 1.0) != 0.0) {
        printf("TypeError: Line-%d\nThe second level \"range\" argument only accepts the \"Number\" type(integer)\n",
        ctx->pos);
        exit(1);
    }

    if (setValue.numberValue > ctx->arg.numberValue) {
        printf("RangeError: Line-%d\nThere must be more first level arguments than first level arguments in the \"range\" argument\n",
        ctx->pos);
        exit(1);
    }

    return newListValue(
        rangeList(
            setValue.numberValue,
            ctx->arg.numberValue,
            ctx->pos,
            ctx->arena
        ),
        ctx->arena
    );
}

Value builtinMap(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_FUNCTION
        && ctx->arg.type != VALUE_BUILTINFUNCTION
        && ctx->arg.type != VALUE_CLANGFUNCTION) {
        printf("TypeError: Line-%d\nThe first level \"map\" argument accepts the \"Function\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(mapSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value mapSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_LIST) {
        printf("TypeError: Line-%d\nThe second level \"map\" argument accepts the \"List\" type\n",
        ctx->pos);
        exit(1);
    }
    
    List *beforeList = ctx->arg.listValue;
    List *afterList = NULL;

    while (beforeList != NULL) {
        afterList = listAppend(afterList, apply(setValue, beforeList->head, ctx->env, ctx->pos, ctx->arena), ctx->arena);
        beforeList = beforeList->tail;
    }

    return newListValue(afterList, ctx->arena);
}

Value builtinForeach(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_FUNCTION
        && ctx->arg.type != VALUE_BUILTINFUNCTION
        && ctx->arg.type != VALUE_CLANGFUNCTION) {
        printf("TypeError: Line-%d\nThe first level \"foreach\" argument accepts the \"Function\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(foreachSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value foreachSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_LIST) {
        printf("TypeError: Line-%d\nThe second level \"foreach\" argument accepts the \"List\" type\n",
        ctx->pos);
        exit(1);
    }
    
    List *list = ctx->arg.listValue;

    while (list != NULL) {
        apply(setValue, list->head, ctx->env, ctx->pos, ctx->arena);
        list = list->tail;
    }

    return newUnitValue(ctx->arena);
}

Value builtinToString(BuiltinFnCtx *ctx) {
    char *str = valueToString(
        ctx->arg,
        ctx->arena
    );

    return newStringValue(str, ctx->arena);
}

Value builtinAppend(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_LIST) {
        printf("TypeError: Line-%d\nThe first level \"append\" argument accepts the \"List\" type\n",
        ctx->pos);
        exit(1);
    }

    return newClangFunctionValue(
        newClangFunction(appendSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value appendSecond(Value setValue, BuiltinFnCtx *ctx) {
    List *newList = listAppend(setValue.listValue, ctx->arg, ctx->arena);
    return newListValue(newList, ctx->arena);
}

Value builtinCons(BuiltinFnCtx *ctx) {
    return newClangFunctionValue(
        newClangFunction(consSecond, ctx->arg, ctx->arena),
        ctx->arena
    );
}

Value consSecond(Value setValue, BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_LIST) {
        printf("TypeError: Line-%d\nThe second level \"cons\" argument accepts the \"List\" type\n",
        ctx->pos);
        exit(1);
    }

    List *newList = listCons(setValue, ctx->arg.listValue, ctx->arena);
    return newListValue(newList, ctx->arena);
}

Value builtinHead(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_LIST) {
        printf("TypeError: Line-%d\nThe first level \"head\" argument accepts the \"List\" type\n",
        ctx->pos);
        exit(1);
    }

    Value head = ctx->arg.listValue->head;
    return head;
}

Value builtinTail(BuiltinFnCtx *ctx) {
    if (ctx->arg.type != VALUE_LIST) {
        printf("TypeError: Line-%d\nThe first level \"tail\" argument accepts the \"List\" type\n",
        ctx->pos);
        exit(1);
    }

    List *newList = ctx->arg.listValue->tail;
    return newListValue(newList, ctx->arena);
}

BuiltinEntry builtinFns[18] = {
    {"println", builtinPrintln},
    {"add", builtinAdd},
    {"sub", builtinSub},
    {"mul", builtinMul},
    {"div", builtinDiv},
    {"mod", builtinMod},
    {"if", builtinIf},
    {"equal", builtinEqual},
    {"let", builtinLet},
    {"fn", builtinFn},
    {"range", builtinRange},
    {"map", builtinMap},
    {"foreach", builtinForeach},
    {"toString", builtinToString},
    {"append", builtinAppend},
    {"cons", builtinCons},
    {"head", builtinHead},
    {"tail", builtinTail}
};

Environment *createGlobalEnvironment(Arena *arena) {
    Environment *env = newEnvironment(NULL, arena);

    // ビルトイン関数を定義する
    for (int i = 0; i < 18; i++) {
        Value fn = newBuiltinFnValue(builtinFns[i].fn, arena);

        define(env, builtinFns[i].name, fn, arena);
    }

    // 定数も定義
    Value trueValue = newBoolValue(true, arena);
    define(env, "true", trueValue, arena);

    Value falseValue = newBoolValue(false, arena);
    define(env, "false", falseValue, arena);

    return env;
}

// 文字列関係
char *valueToString(Value val, Arena *arena) {
    char buffer[64];

    switch (val.type) {

        case VALUE_NUMBER:
            snprintf(
                buffer,
                sizeof(buffer),
                "%g",
                val.numberValue
            );

            return arenaStrdup(arena, buffer);


        case VALUE_STRING: {
            snprintf(
                buffer,
                sizeof(buffer),
                "%s",
                val.stringValue
            );

            return arenaStrdup(arena, buffer);
        }


        case VALUE_BOOL:
            return arenaStrdup(
                arena,
                val.boolValue ? "true" : "false"
            );


        case VALUE_UNIT:
            return arenaStrdup(arena, "<unit>");


        case VALUE_FUNCTION:
            return arenaStrdup(arena, "<function>");


        case VALUE_BUILTINFUNCTION:
        case VALUE_CLANGFUNCTION:
            return arenaStrdup(arena, "<builtinFunction>");


        case VALUE_LIST:
            return listToString(
                val.listValue,
                arena
            );


        case VALUE_QUOTE:
            return arenaStrdup(arena, "<quote>");


        default:
            return arenaStrdup(arena, "<unknown>");
    }
}

char *listToString(List *list, Arena *arena) {
    char *result = arenaAlloc(arena, 4096);

    result[0] = '\0';

    strcat(result, "[");

    while (list != NULL) {

        char *element =
            valueToString(list->head, arena);

        strcat(result, element);

        if (list->tail != NULL) {
            strcat(result, ", ");
        }

        list = list->tail;
    }

    strcat(result, "]");

    return result;
}

// リスト関係
List *rangeList(int start, int end, int pos, Arena *arena) {
    if (start > end) {
        return NULL;
    }

    List *list = arenaAlloc(arena, sizeof(List));

    list->head.type = VALUE_NUMBER;
    list->head.numberValue = start;

    list->tail = rangeList(
        start + 1,
        end,
        pos,
        arena
    );

    return list;
}

List *listCons(Value value, List *list, Arena *arena) {
    List *newList = arenaAlloc(arena, sizeof(List));

    newList->head = value;
    newList->tail = list;

    return newList;
}

List *listAppend(List *list, Value value, Arena *arena) {
    if (list == NULL) {
        return listCons(value, NULL, arena);
    }

    List *newList = arenaAlloc(arena, sizeof(List));

    newList->head = list->head;
    newList->tail = listAppend(list->tail, value, arena);

    return newList;
}

// value
Value newNumberValue(double num, Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_NUMBER;
    newValue->numberValue = num;
    return *newValue;
}

Value newStringValue(char *str, Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_STRING;
    newValue->stringValue = str;
    return *newValue;
}

Value newBoolValue(bool bool_, Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_BOOL;
    newValue->boolValue = bool_;
    return *newValue;
}

Value newUnitValue(Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_UNIT;
    return *newValue;
}

Value newFunctionValue(Function func, Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_FUNCTION;
    newValue->functionValue = func;
    return *newValue;
}

Value newBuiltinFnValue(Value (*builtinFnValue)(BuiltinFnCtx *ctx), Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_BUILTINFUNCTION;
    newValue->builtinFnValue = builtinFnValue;
    return *newValue;
}

Value newClangFunctionValue(ClangFunction cfunc, Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_CLANGFUNCTION;
    newValue->clangFnValue = cfunc;
    return *newValue;
}

Value newListValue(List *list, Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_LIST;
    newValue->listValue = list;
    return *newValue;
}

Value newQuoteValue(Node *quote, Arena *arena) {
    Value *newValue = arenaAlloc(arena, sizeof(Value));
    newValue->type = VALUE_QUOTE;
    newValue->quoteValue = quote;
    return *newValue;
}