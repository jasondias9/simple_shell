#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define ARG_MAX 100 /* Max char length in sh is actually 2097152. Reduced
                        to reduced here for simplification */
#define MAX_BG 10 
#define PROMPT "jsh>> "
int BG_COUNT = 0;


typedef struct job {
    pid_t pid;
    char cmd[ARG_MAX];

} job;

struct job jobs[MAX_BG];


int getcmd(char *args[], int *background, char *line) {
    int length, i = 0;
    char *token, *loc;
    size_t linecap = 0;
    
    printf("%s", PROMPT);
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

    token = strtok(line, " \t\n");

    while(token != NULL) {
        for(int j = 0; j < strlen(token); j++) {
            if(token[j] <= 32) token[j] = '\0';
        }
        if(strlen(token) > 0) {
            args[i++] = token;
        }
        
        token = strtok(NULL, " \t\n"); 
    }
    

    return i;
}

int execute(char *line, char *args[], int bg) {
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
    } else {


    }

    return 0;
}

int listjobs() {

}

int main(void) {
    
    char *args[20];
    char line[ARG_MAX];
    int bg;

    while(1) {
        bg = 0;
        fflush(stdout);
        memset(args, 0, ARG_MAX);
        memset(line, 0, ARG_MAX);

        int cnt = getcmd(args, &bg, line);
        fflush(stdout);


        if(strcmp(args[0], "pwd") == 0) {
            char path[ARG_MAX];
            if(getcwd(path, ARG_MAX) != NULL) {
                printf("%s\n", path);
                continue;
            }
        } else if(strcmp(args[0], "cd") == 0) {
            chdir(args[1]);
            continue;
        } else if(strcmp(args[0], "exit") == 0) {
            exit(0);
        } else if(strcmp(args[0], "fg") == 0) {
            pid_t pid = jobs[atoi(args[1]) + 1].pid;
            waitpid(pid, NULL, 0);
        } else if(strcmp(args[0], "jobs") == 0) {
           listjobs(); //TODO : implement helper func to list jobs
        } else {
            execute(line, args, bg);
        }


    }
}
