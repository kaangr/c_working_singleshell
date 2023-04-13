#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define PROMPT "$ "
#define PWD "/bin/ls"
char inbuf[256] = {'\0'};

int writeToFile() {
    FILE *fp = fopen("log.txt", "a");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }
    if (fwrite(inbuf, sizeof(char), strlen(inbuf), fp) != strlen(inbuf)) {
        perror("fwrite");
        fclose(fp);
        return 1;
    }
    if (fwrite("\n", sizeof(char), 1, fp) != 1) {
        perror("fwrite");
        fclose(fp);
        return 1;
    }
    fclose(fp);
    return 0;
}

int main() {
    size_t nbyte;
    printf("myshell id: %d\n", getpid());
    while (1) {
        write(STDOUT_FILENO, PROMPT, 2);

        if ((nbyte = read(STDIN_FILENO, inbuf, 255)) <= 0) {
            perror("read");
            continue;
        }
        inbuf[nbyte - 1] = '\0';
        char cmd[256];
        strcpy(cmd, PWD);
        strcat(cmd, inbuf);
        if (strlen(cmd) >= 256) {
            perror("command too long");
            continue;
        } else {
            int ret = fork();
            if (ret == -1) {
                perror("fork");
                continue;
            } else if (ret == 0) {
                execl(cmd, PWD, NULL);
                perror("execl");
                exit(1);
            } else {
                wait(NULL);
            }
        }

        if (strcmp(inbuf, "exit") == 0) {
            break;
        }
        if (writeToFile() != 0) {
            perror("writeToFile");
        }
    }

    return 0;
}