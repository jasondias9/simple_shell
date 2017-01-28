#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define ARG_MAX 1024
#define MAX_JOBS 10 
#define PROMPT "jsh>> "


int BG_COUNT = 0;

typedef struct job {
    int jid;
    pid_t pid;
    int state;
    char cmd[ARG_MAX];

} job;

struct job jobs[MAX_JOBS];

/* Prototypes */
int get_cmd(char *args[], int *background, int length, char *line);
int execute(char *args[], int bg, char *line);
int list_jobs();
int merge_cmd(char **cmd, char *args[], int size);



int get_cmd(char *args[], int *background, int length, char *line) {
    int i = 0;
    char *token, *loc;

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

int execute(char *args[], int bg, char *line) {
    pid_t pid = fork();
    int status;
    if(pid < 0) {
        printf("**** ERROR: fork failed\n");
        exit(1);
    } else if(pid == 0) {
        if(execvp(args[0], args) < 0) { 
            printf("jsh: command not found :  %s\n", args[0]); //assumumption
            exit(1);
        } 
    }
    if(!bg) {
        while(wait(&status) != pid);
    } else {
       struct job n_job;
       n_job.pid = pid;
       strcpy(n_job.cmd, line);
       n_job.jid = BG_COUNT+1;
       jobs[BG_COUNT] = n_job;
       BG_COUNT++;
       printf("[%i]   %i\n",BG_COUNT, (int)pid);
    }

    return 0;
}

int list_jobs() {
    int i;
    for(i = 0; i < BG_COUNT; i++) {
        printf("[%i]  +  %s\n", jobs[i].jid, jobs[i].cmd);
    } 
}

int merge_cmd(char **cmd, char *args[], int size) {
    int i;
    for(i = 0; i < size; i++){
        strcat(*cmd, args[i]);
        strcat(*cmd, " ");
    }
    return 0;
}


/*Main Routine*/

int main(void) {
    
    char *args[20];
    int bg;

    while(1) {
        bg = 0;
        fflush(stdout);
        memset(&args, 0, ARG_MAX);
        
        char *line;
        size_t linecap = 0;
        int length = 0;

        printf("%s", PROMPT);
        length = getline(&line, &linecap, stdin);
        int cnt = get_cmd(args, &bg, length, line);
        char *cmd = malloc(length);
        merge_cmd(&cmd, args, cnt);
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
           list_jobs();
        } else {
            execute(args, bg, cmd);
            free(cmd);
        }
    }
}
