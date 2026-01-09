#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

// 位置からエラーを出す
void errorAtPos(char *msg, int linePos, char *source) {
    printf("Error: %s\n", msg);
    printf(" at line: %d\n\n", linePos);
    char *line = getLineFromPos(linePos, source);
    printf("%s\n", line);

    size_t len = strlen(line);
    char *caret = malloc(len + 1);
    if (!caret) exit(1);
    for (size_t i = 0; i < len; i++) {
        caret[i] = '^';
    }
    caret[len] = '\0';

    printf("%s\n", caret);
    exit(1);
}

char *getLineFromPos(int linePos, char *source) {
    char *line = strtok(source, "\r\n");
    int numOfNowLine = 1;
    while (line != NULL) {
        if (linePos == numOfNowLine) {
            return line;
        }

        numOfNowLine++;
        line = strtok(NULL, "\r\n");
    }
    
    printf("akirametekudasai");
    exit(1);
}
