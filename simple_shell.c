#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define ARG_MAX 100 /* Max char length in sh is actually 2097152. Reduced
                        to reduce impact */

int getcmd(char *prompt, char *args[], int *background) {
    int length, i = 0;
    char *token, *loc;
    char *line = NULL;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    if(length <= 0) {
        exit(-1);
    }

    if((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else {
        *background = 0;
    }

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for(int j = 0; j < strlen(token); j++) {
            if(token[j] <= 32) {
                token[j] = '\0';
            }
            if(strlen(token) > 0) {
                args[i++] = token;
            }
        }
    }    
    return i;
}

int execute(char *args[], int bg) {
    pid_t pid = fork();
    int status;
    if(pid < 0) {
        printf("**** ERROR: fork failed");
        exit(1);
    } else if(pid == 0) {
        if(execvp(args[0], args) < 0) {
            printf("**** ERROR: execution of %s failed", args[0]);
            exit(1);
        } 
    }
    if(!bg) {
        while(wait(&status) != pid);
        printf("%i", (int)pid);
    }
}



int main(void) {
    
    char *args[20];
    int bg;

    while(1) {
        bg = 0;
        int cnt = getcmd("\njsh >> ", args, &bg);
        if(strcmp(args[0], "pwd") == 0) {
            char path[ARG_MAX];
            if(getcwd(path, ARG_MAX) != NULL) {
                printf("%s", path);
                continue;
            } 
        } else if(strcmp(args[0], "cd") == 0) {
            chdir(args[1]);
            continue;
        } else if(strcmp(args[0], "exit") == 0) {
            exit(0);
        } else if(strcmp(args[0], "fg") == 0) {
            pid_t pid = jobs[atoi(args[1]) + 1].pid; //TODO : maintain jobs ds
            waitpid(pid, NULL, 0); /* bring to the front */
        } else if(strcmp(args[0], "jobs") == 0) {
           listJobs(); //TODO : implement helper func to list jobs
        }
        execute(args, bg);
    }
}
