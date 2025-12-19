#include <stdio.h>
#include <stdlib.h>
#include "tokenizer/tokenizer.h"
#include "parser/parser.h"

char *readFile(const char *filepath);

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
    char *code = readFile(filepath);
    
    printf("====code====\n%s\n========\n", code);

    TokenList tokens = tokenizer(code);

    printTokens(&tokens);

    ASTNode node = parser(&tokens);
    printAST(&node, 0);

    // free
    free(code);
    freeTokenList(&tokens);
    freeAST(&node);

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
