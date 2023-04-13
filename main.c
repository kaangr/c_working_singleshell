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
char inbuf[256] = {'\0'};
#define PROMPT "$ "
enum CustomError
{
    _FWRITE = 1,
    _FILE = 2,
    _FORK = 3,
    _EXEC = 4,
    _WRITETOFILE = 5
};
enum CustomError err;

char *Err_string(int value)
{
    char *err_str;
    switch (value)
    {

    case _FWRITE:
        err_str = "err:  _FWRITE";
        break;
    case _FILE:
        err_str = "err:  _FILE";
        break;
    case _FORK:
        err_str = "err:  _FORK";
        break;
    case _EXEC:
        err_str = "err:  _EXEC";
        break;
    case _WRITETOFILE:
        err_str = "err:  _WRITETOFILE";

    default:
        err_str = "er:    Unknown error";
        break;
    }
    // err_str = malloc(sizeof(strlen(err_str+1)));
    err_str = (char *)malloc(20 * sizeof(char));

    return err_str;
}

int writeErrors(int err_value)
{
    FILE *fp = fopen("log.txt", "a");
    if (fp == NULL)
    {
        perror("file error");
        writeErrors(2);

        fclose(fp);
        return 1;
    }
    char *err_str = Err_string(err_value);
    fwrite(err_str, sizeof(char), sizeof(err_str), fp);

    if (fwrite("\n", sizeof(char), 1, fp) != 1)
    {
        perror("fwrite");
        fclose(fp);
        return 1;
    }
    fclose(fp);

    return 0;
}

int writeToFile()

{
    printf("\nWRITETOFILE EXECUTED\n");
    FILE *fp = fopen("log.txt", "a");
    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }
    if (fwrite(inbuf, sizeof(char), strlen(inbuf), fp) != strlen(inbuf))
    {
        perror("fwrite");
        fclose(fp);
        return 1;
    }
    if (fwrite("\n", sizeof(char), 1, fp) != 1)
    {
        perror("fwrite");
        writeErrors(1);
        fclose(fp);
        return 1;
    }
    fclose(fp);
    return 0;
}

int main()
{

    size_t nbyte;
    printf("myshell id: %d\n", getpid());
    while (1)
    {
        fflush(stdout);
        write(STDOUT_FILENO, PROMPT, 2);

        if ((nbyte = read(STDIN_FILENO, inbuf, 255)) <= 0)
        {
            perror("read");
            writeErrors(2);
            continue;
        }
        inbuf[nbyte - 1] = '\0';

        pid_t child_pid = fork();
        if (child_pid == -1) // error
        {
            perror("fork");
            writeErrors(3);
            continue;
        }

        else if (child_pid == 0) // child process
        {

            printf("parentes ppid = %d child process id =%d\n", getppid(), getpid());
            int r = execl(inbuf, inbuf, NULL); // second was PWD so used to  added "bin/ls/"

            if (r == -1)
            {
                char command[255] = {'/', 'b', 'i', 'n', '/', '\0'};

                strncat(command, inbuf, 250);
                strcpy(inbuf, command); // for writing to log.txt file
                if (strlen(command) > 256)
                {
                    perror("command too long");
                    continue;
                }

                int r = execl(command, command, NULL);

                if (r == -1)
                {
                    perror("execl");
                    writeErrors(4);
                }
            }

            else
            {
                wait(NULL);
            }
        }

        if (writeToFile() != 0)
        {
            perror("writeToFile");
            writeErrors(1);
        }
        if (strcmp(inbuf, "exit")==0)
        {
            printf("exiting program.\n");
            exit(1);
        }
    }
    return 0;
}