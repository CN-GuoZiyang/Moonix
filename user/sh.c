/*
 *  user/sh.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "ulib.h"
#include "syscall.h"

#define LF      0x0au
#define CR      0x0du
#define BS      0x08u
#define DL      0x7fu
#define CTRLC   0x03u

int
isEmpty(char *line, int length) {
    int i;
    for(i = 0; i < length; i ++) {
        if(line[i] == 0) break;
        if(line[i] != ' ' && line[i] != '\t') {
            return 0;
        }
    }
    return 1;
}

void
empty(char *line, int length)
{
    int i;
    for(i = 0; i < length; i ++) {
        line[i] = 0;
    }
}

int
isBuildIn(char *line, int fd) {
    if(!strcmp("shutdown", line)) {
        sys_shut();
        return 1;
    }
    int len = strlen(line);
    /* 处理 ls */
    if(len >= 2 && line[0] == 'l' && line[1] == 's' && (line[2] == ' ' || line[2] == '\t' || line[2] == '\0')) {
        line += 3;
        while(*line == ' ' || *line == '\t') line ++;
        sys_lsdir(line, fd);
        return 1;
    }
    /* 处理 cd */
    if(len >= 2 && line[0] == 'c' && line[1] == 'd' && (line[2] == ' ' || line[2] == '\t' || line[2] == '\0')) {
        line += 3;
        while(*line == ' ' || *line == '\t') line ++;
        if(*line == 0) {
            printf("cd: need a path\n");
            return 1;
        }
        sys_cddir(line, fd);
        return 1;
    }
    if(len >= 3 && line[0] == 'p' && line[1] == 'w' && line[2] == 'd' && (line[3] == ' ' || line[3] == '\t' || line[3] == '\0')) {
        sys_pwd(fd);
        return 1;
    }
    return 0;
}

uint64
main()
{
    char line[256];
    int lineCount = 0;
    int fd = sys_open("/");
    printf("Welcome to Moonix!\n");
    printf("$ ");
    while(1) {
        uint8 c = getc();
        switch(c) {
            case LF: case CR: 
                printf("\n");
                if(!isEmpty(line, 256)) {
                    char *stripLine = line;
                    while(*stripLine == ' ' || *stripLine == '\t') stripLine ++;
                    if(!isBuildIn(stripLine, fd)) {
                        sys_exec(stripLine, fd);
                    }
                    lineCount = 0;
                    empty(line, 256);
                }
                printf("$ ");
                break;
            case CTRLC:
                printf("\n");
                lineCount = 0;
                empty(line, 256);
                printf("$ ");
                break;
            /* 支持退格键 */
            case DL:
                if(lineCount > 0) {
                    putchar(BS);
                    putchar(' ');
                    putchar(BS);
                    line[lineCount-1] = 0;
                    lineCount -= 1;
                }
                break;
            default:
                if(lineCount < 255) {
                    line[lineCount] = c;
                    lineCount += 1;
                    putchar(c);
                }
                break;
        }
    }
}