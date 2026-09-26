#ifndef _EVALUATOR_H_
#define _EVALUATOR_H_

typedef enum {
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_BOOL,
    VALUE_UNIT,
    VALUE_FUNCTION,
    VALUE_BUILTINFUNCTION,
    VALUE_CLANGFUNCTION,
    VALUE_LIST,
    VALUE_QUOTE
} ValueType;

typedef struct Value Value;

// functionで使うために早めにやっとく
typedef struct Environment Environment;

typedef struct {
    Node *body;
    char *param;
    Environment *parentEnv;
} Function;

// BuiltinFnCtxを他のところでも使うため早めにやっとく
typedef struct BuiltinFnCtx BuiltinFnCtx;

typedef struct {
    char *name;
    Value (*fn)(BuiltinFnCtx *ctx);
} BuiltinEntry;

typedef struct {
    Value *setValue;
    Value (*clangFnValue)(Value setValue, BuiltinFnCtx *ctx);
} ClangFunction;

typedef struct List List;

struct Value {
    ValueType type;
    union {
        double numberValue;
        char *stringValue;
        bool boolValue;
        Function functionValue;
        Value (*builtinFnValue)(BuiltinFnCtx *ctx);
        // ビルトイン関数レベルの処理をしたいが
        // その処理をする関数のValueを返さなければいけないときに使う
        ClangFunction clangFnValue;
        List *listValue;
        Node *quoteValue;
    };
};

struct List {
    Value head;
    struct List *tail;
};

typedef struct {
    char *name;
    Value value;
} Constant;

struct Environment {
    Constant *constants;
    size_t count;
    size_t capacity;
    Environment *parent; // 定数の総数（定数を取り出すときに必要）
};

Value evaluator(Node *node, Environment *env, Arena *arena);
Value apply(Value func, Value arg, Environment *env, int pos, Arena *arena);
Value literalToValue(Node *node, Environment *env, Arena *arena);
Value getValueFromIdent(Node *node, Environment *env);

// environment関係
Environment *newEnvironment(Environment *parent, Arena *arena);
void define(Environment *env, char *name, Value value, Arena *arena);

// ビルトイン関数を定義
struct BuiltinFnCtx {
    Value arg;
    Environment *env;
    int pos;
    Arena *arena;
};
BuiltinFnCtx *newBuiltinFnCtx(Value arg, Environment *env, int pos, Arena *arena);

Value makeClangFunction(ClangFunction clangFnValue);
ClangFunction newClangFunction(Value (*clangFnValue)(Value setValue, BuiltinFnCtx *ctx), Value setValue, Arena *arena);
Value builtinPrintln(BuiltinFnCtx *ctx);
Value builtinAdd(BuiltinFnCtx *ctx);
Value addSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinSub(BuiltinFnCtx *ctx);
Value subSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinMul(BuiltinFnCtx *ctx);
Value mulSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinDiv(BuiltinFnCtx *ctx);
Value divSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinMod(BuiltinFnCtx *ctx);
Value modSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinIf(BuiltinFnCtx *ctx);
Value ifSecond(Value setValue, BuiltinFnCtx *ctx);
Value ifThird(Value setValue, BuiltinFnCtx *ctx);
Value ifFourth(Value setValue, BuiltinFnCtx *ctx);
Value builtinEqual(BuiltinFnCtx *ctx);
Value equalSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinLet(BuiltinFnCtx *ctx);
Value letSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinFn(BuiltinFnCtx *ctx);
Value fnSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinRange(BuiltinFnCtx *ctx);
Value rangeSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinMap(BuiltinFnCtx *ctx);
Value mapSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinForeach(BuiltinFnCtx *ctx);
Value foreachSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinToString(BuiltinFnCtx *ctx);
Value builtinAppend(BuiltinFnCtx *ctx);
Value appendSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinCons(BuiltinFnCtx *ctx);
Value consSecond(Value setValue, BuiltinFnCtx *ctx);
Value builtinHead(BuiltinFnCtx *ctx);
Value builtinTail(BuiltinFnCtx *ctx);
Environment *createGlobalEnvironment(Arena *arena);

// 文字列関係
char *valueToString(Value val, Arena *arena);
char *listToString(List *list, Arena *arena);

// リスト関係
List *rangeList(int start, int end, int pos, Arena *arena);
List *listCons(Value value, List *list, Arena *arena);
List *listAppend(List *list, Value value, Arena *arena);

#endif