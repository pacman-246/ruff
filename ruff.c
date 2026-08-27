#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arena/arena.h"
#include "tokenizer/tokenizer.h"
#include "parser/parser.h"
#include "evaluator/evaluator.h"

char *readFile(const char *filepath);
char *normalizeNewlines(const char *src);

int main(int argc, char *argv[]) {
    // コマンドの間違いがないか確認
    if (argc != 2 && argc != 3) {
        printf("Usage: %s <filename> (-debug)\n", argv[0]);
        return 1;
    }

    bool debugFlag = false;
    if (argc == 3 && strcmp(argv[2], "-debug") == 0) {
        debugFlag = true;
    } else if (argc == 3 && strcmp(argv[2], "-debug") != 0) {
        printf("CommandError: only the \"-debug\" option is implemented");
        return 1;
    }

    // アリーナの初期化
    Arena arena;
    arenaInit(&arena);

    char *code = readFile(argv[1]);
    char *normalizedCode = normalizeNewlines(code);
    if (debugFlag) {
        printf("====code====\n%s\n============\n", normalizedCode);
    }

    free(code); // readFileでmallocした一時バッファは解放

    // 字句解析
    TokenList *tokenList = tokenizer(normalizedCode, &arena);
    if (debugFlag) {
        printf("===tokens===\n");
        printTokenList(tokenList);
        printf("============\n");
    }

    // 構文解析
    Node *node = parser(tokenList, &arena);
    if (debugFlag) {
        printf("====node====\n");
        printNode(node, 0);
        printf("============\n");
    }

    // 評価、実行
    // 新しい環境を用意
    Environment *globalEnv = createGlobalEnvironment(&arena);
    if (debugFlag) {
        printf("====eval====\n");
        evaluator(node, globalEnv, &arena);
        printf("============\n");
    } else {
        evaluator(node, globalEnv, &arena);
    }

    // アリーナの解放（すべてのメモリを一括解放）
    arenaFree(&arena);
    free(normalizedCode); // normalizeNewlinesでmallocした一時バッファは解放

    return 0;
}

//ファイルを読み込む関数
char *readFile(const char *filepath) {
    //ファイルを開く
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        printf("FileError: the file specified in the command could not be opened\n");
        exit(1);
    }

    //ファイルサイズを取得
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    
    //メモリ確保
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        printf("FileError: the file specified in the command was too large to allocate memory\n");
        exit(1);
    }

    //読み込む
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';

    fclose(fp);
    return buffer;
}

// 改行をすべて\nにする関数
char *normalizeNewlines(const char *src) {
    size_t len = strlen(src);

    // 改行は減ることはあっても増えないので同じ長さで十分
    char *dst = malloc(len + 1);
    if (!dst) return NULL;

    size_t i = 0, j = 0;

    while (src[i]) {
        if (src[i] == '\r') {
            dst[j++] = '\n';
            if (src[i + 1] == '\n') {
                i++; // \r\n の \n を飛ばす
            }
        } else {
            dst[j++] = src[i];
        }
        i++;
    }

    dst[j] = '\0';
    return dst;
}