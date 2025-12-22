#include <stdio.h>
#include <stdlib.h>
#include "tokenizer/tokenizer.h"
#include "parser/parser.h"
#include "run/run.h"

char *readFile(const char *filepath);
char *replaceTabWithSpaces(const char *src);

//------------------------メイン------------------------

int	main(int argc, char *argv[]) {
    //コマンドの形はruff -mode filepathなのでargcが3じゃなかった場合はエラー
    if (argc != 3) {
        printf("CommandError: the command format is incorrect\n");
        return 1;
    }

    //ファイルを読み込む
    char *filepath = argv[2];
    //33行を参照
    char *rawCode = readFile(filepath);
    
    printf("====code====\n%s\n========\n", rawCode);

    // トークナイザーで認識しやするするために\tを半角空白四文字に変換
    char *code = replaceTabWithSpaces(rawCode);

    TokenList tokens = tokenizer(code);

    printTokens(&tokens);

    Arena arena;
    arenaInit(&arena, 1024 * 1024); // 1MB

    ASTNode node = parser(&arena, &tokens);
    printAST(&node, 0);

    run(&node);

    // free
    free(code);
    freeTokenList(&tokens);
    arenaFree(&arena);

    return 0;
}

//------------------------関数------------------------

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

char *replaceTabWithSpaces(const char *src) {
    int i, len = 0;

    // 必要な長さを計算
    for (i = 0; src[i] != '\0'; i++) {
        if (src[i] == '\t')
            len += 4;
        else
            len += 1;
    }

    // メモリ確保
    char *dst = malloc(len + 1);
    if (!dst) return NULL;

    // 変換
    int j = 0;
    for (i = 0; src[i] != '\0'; i++) {
        if (src[i] == '\t') {
            dst[j++] = ' ';
            dst[j++] = ' ';
            dst[j++] = ' ';
            dst[j++] = ' ';
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';

    return dst;
}
