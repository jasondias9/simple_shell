#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

#define ARG_MAX 1024
#define MAX_JOBS 10 
#define PROMPT "[jsh]>> "


int BG_COUNT = 0;
typedef struct job {
    int jid;
    pid_t pid;
    char state[10];
    char cmd[ARG_MAX];

} job;

struct job jobs[MAX_JOBS];
int fg_job; 

/* Function Prototypes */
int get_cmd(char *args[], int *background, int *out, int *pip, int length, char *line);
int execute(char *args[], int cnt, int bg, int out, int pip, char *line);
int list_jobs();
int merge_cmd(char **cmd, char *args[], int size);
int slice_array(char *args[], char *args_c[], int start, int end);
int execute_pipe(char *args[], int cnt);
int check_exec_built_in(char *args[]);


/* Parse and tokenize input from user */
int get_cmd(char *args[], int *background, int *out, int *pip, int length, char *line) {
    fflush(stdout);
    int pip_i, i = 0;
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
    
    if((loc = index(line, '>')) != NULL) {
        *loc = ' ';
        *out = 1;
    } else {
        out = 0;
    }
 
    if((loc = index(line, '|')) != NULL) {
        *pip = 1;
    } else {
        pip = 0;
    }
    token = strtok(line, " \t\n");

    while(token != NULL) {
        for(int j = 0; j < strlen(token); j++) { 
            if(token[j] <= 32) token[j] = '\0'; 
            if(token[j] == 124) {
                token[j] = '\0';
                pip_i = i;
            }
        }

        if(strlen(token) > 0) {
            args[i++] = token;
        }
        token = strtok(NULL, " \t\n"); 
    }
    if(pip) {
        i = pip_i; 
    }
    return i;
}

/* Provide a slice function for array generation in piping */
int slice_array(char *args[], char *args_c[], int start, int end) {
    int i;
    for(i = 0; i+start < end; i++) {
        args_c[i] = args[i+start];
        if(i+1 == end) {
            args_c[i+1] = NULL;
        }
    } 
    args_c[i+1] = NULL;
}

/* Execute piping within a helper execute method */
int execute_pipe(char *args[], int cnt) {
    int pipefd[2];
    if(pipe(pipefd) == -1) {
        perror("piping error");
    }
    pid_t pid = fork();
    if(pid == 0) {
        char *args1[20];
        slice_array(args, args1, 0, cnt);
        close(pipefd[0]);
        dup2(pipefd[1], fileno(stdout));
        close(pipefd[1]);
        execvp(args1[0], args1);
        perror("exec error: ");
        exit(1);
    }
    char *args2[20];
    slice_array(args, args2, cnt, 19);
    close(pipefd[1]);
    dup2(pipefd[0], fileno(stdin));
    close(pipefd[0]);
    execvp(args2[0], args2);
    perror("exec error: ");
    exit(1);
}

/* Add a background job to jobs */
int add_job(pid_t pid, char * line) {
    struct job n_job;
    n_job.pid = pid; 
    strcpy(n_job.cmd, line);
    strcpy(n_job.state, "running"); //add additional states in the future
    for(int i = 0; i < MAX_JOBS; i++) {
        if(jobs[i].pid == 0) {
            n_job.jid = i+1;  
            jobs[i] = n_job;
            BG_COUNT = i+1;
            printf("[%i] %i\n", BG_COUNT, (int)pid);
            break;
        }
    }
}

/* Execute the cmd and arguments */
int execute(char *args[], int cnt, int bg, int out, int pip, char *line) {
    pid_t pid = fork();
    int status;
    if(pid < 0) {
        printf("**** ERROR: fork failed\n");
        exit(1);
    } else if(pid == 0) {
        //Child
        if(out) {
            close(fileno(stdout));
            int fd1;
            fd1 = open(args[cnt-1], O_RDWR|O_CREAT, 0777);
            args[cnt-1] = 0; 
        } else if(pip) {
           execute_pipe(args, cnt);
        }
        if(execvp(args[0], args) < 0) { 
            printf("jsh: command not found :  %s\n", args[0]);//assumumption
            exit(1);
        } 
        fflush(stdout);
    }
    //Parent
    if(!bg) {
        fg_job = pid;
        while(wait(&status) != pid);
        fg_job = 0;
    } else {
        add_job(pid, line);
    }
    return 0;
}

/* List background jobs */
int list_jobs() {
    int i;
    for(i = 0; i < BG_COUNT; i++) {
        if(jobs[i].pid != 0) {
            printf("[%i]   %s  %s\n", jobs[i].jid, jobs[i].state, jobs[i].cmd);
        }
    } 
}

/* Generate actual command input from user without '&' */
int merge_cmd(char **cmd, char *args[], int size) {
    int i;
    for(i = 0; i < size; i++){
        strcat(*cmd, args[i]);
        strcat(*cmd, " ");
    }
    return 0;
}

/*Check if the cmd is inbuilt, and execute*/
int check_exec_built_in(char *args[]) {
    if(strcmp(args[0], "pwd") == 0) {
        char path[ARG_MAX];
        if(getcwd(path, ARG_MAX) != NULL) {
            printf("%s\n", path);
            return 1;
        }
    } else if(strcmp(args[0], "cd") == 0) {
        chdir(args[1]);
        return 1;
    } else if(strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if(strcmp(args[0], "fg") == 0) {
        if(args[1]) {
            pid_t pid = jobs[atoi(args[1]) + 1].pid;
            if(pid < 0) {
                printf("fg: job not found: %s\n", args[1]);
            }
            fg_job = pid;
            waitpid(pid, NULL, 0);
            fg_job = 0;
            return 1;
        } else {
            printf("fg: no current job\n");
            return 1;
        }
    } else if(strcmp(args[0], "jobs") == 0) { 
        list_jobs();
        return 1;
    } else if(strcmp(args[0], "clear") == 0) {
        //the external clear cmd didnt always work. Did this for convenience.  
        printf("\033[2J\033[1;1H"); 
        return 1;
    }
    return 0;
}

int handle_completed_bg_job() {
    for(int j = 0; j < MAX_JOBS; j++) {
        if(jobs[j].pid != 0) {
            int status;
            pid_t pid =  waitpid(jobs[j].pid, &status, WNOHANG);
            if(pid < 0) {
                //process probably doesn't exist anymore
                struct job n_job;
                n_job.pid = 0;
                jobs[j] = n_job;
            } else if(pid == 0) {
                //do nothing, proc still running
                
            } else if(pid = jobs[j].pid) {
                struct job n_job;
                n_job.pid = 0;
                jobs[j] = n_job;
            }
        }
    }
}

void interrupt_handler(int signo) {
    if(fg_job != 0) {
        printf("foreground");
        kill(fg_job, SIGKILL);
    } else {
        //somehow replicate a 'continue' in while loop.
    }
}

/*Main Routine*/
int main(void) { 
    char *args[20];
    int bg;
    int out;
    int pip;

    if(signal(SIGINT, interrupt_handler) == SIG_ERR) {
      perror("somethign went wrong"); 
    }

    while(1) {
        
        bg = 0;
        out = 0;
        pip = 0;
        fflush(stdout); 
        memset(&args, 0, ARG_MAX);

        char *line;
        size_t linecap = 0;
        int length = 0;
        printf("%s", PROMPT);
         
        handle_completed_bg_job(); 
        
        length = getline(&line, &linecap, stdin);

        //Avoid seg-fault on empty input
        if (length == 1) {
            continue;
        }

        int cnt = get_cmd(args, &bg, &out, &pip, length, line);
        fflush(stdout);
        char *cmd;

        if(bg) {
            cmd = malloc(length);
            merge_cmd(&cmd, args, cnt);
        }
        
        int ran = 0;
        ran = check_exec_built_in(args);
        if(ran) continue;        
        else {
            execute(args, cnt, bg, out, pip, cmd);
            if(bg) {
                free(cmd);
            } 
            free(line);
        }
    }
}
