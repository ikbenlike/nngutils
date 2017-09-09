#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define SH_PROMPT "nngsh-1.0$ "

extern char **environ;

struct sh_args {
    bool interactive;
    bool standard_input;
    bool has_command;
    char *command;
};

struct sh_command {
    int out;
    int in;
    char **argv;
    int argc;
};

void sigint_handler(int sig){
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, SH_PROMPT, strlen(SH_PROMPT));
    return;
}

struct sh_args *parse_arg(struct sh_args *args, int argc, char **argv){
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            if(!strcmp(argv[i], "-c")){
                args->has_command = true;
                if(i + 1 < argc){
                    args->command = argv[++i];
                }
                else {
                    fprintf(stderr, "%s: -c option requires an argument\n", argv[i]);
                }
            }
            else if(!strcmp(argv[i], "-s")){
                args->standard_input = true;
            }
            else if(!strcmp(argv[i], "-i")){
                args->interactive = true;
            }
        }
        else {
            fprintf(stderr, "%s: invalid argument %s\n", argv[0], argv[i]);
        }
    }
    return args;
}

int command_get_argc(char *buffer){
    int n = 0;
    char *p;
    while(p = strchr(buffer, ' ')){
        n++;
        buffer = p + 1;
    }
    return n;
}

struct sh_command *parse_command(char *buffer, size_t len){
    struct sh_command *command = calloc(1, sizeof(struct sh_command));
    command->argc = command_get_argc(buffer) + 1;
    command->argv = calloc(command->argc + 1, sizeof(char*));
    char *token = calloc(len + 2, sizeof(char));
    for(int i = 0, n = 0; i < command->argc && n < len; i++ && n++){
        int x = 0;
        while(buffer[n] != ' ' && buffer[n] != EOF && buffer[n] != '\0'){
            token[x++] = buffer[n++];
        }
        if(i < 1){
            n++;
        }
        token[x++] = '\0';
        command->argv[i] = calloc(x, sizeof(char));
        memcpy(command->argv[i], token, x);
    }
    command->argv[command->argc] = NULL;
    free(token);
    return command;
}

int call_command(struct sh_command *command){
    int status = 0;
    pid_t pid = fork();
    if(pid == -1){
        fprintf(stderr, "sh: %s\n", strerror(errno));
        return 1;
    }
    else if(pid){
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)){
            status = WEXITSTATUS(status);
        }
    }
    else {
        if(execvp(command->argv[0], command->argv) == -1){
            fprintf(stderr, "sh: %s: %s\n", command->argv[0], strerror(errno));
            for(int i = -1; command->argv[++i] != NULL; free(command->argv[i]));
            free(command->argv);
            free(command);
            exit(1);
        }
    }
    return status;
}

int main(int argc, char **argv){
    signal(SIGINT, sigint_handler);
    struct sh_command *command = NULL;
    if(argc > 1){
        struct sh_args *args = calloc(1, sizeof(struct sh_args));
        if(args == NULL){
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        }
        args = parse_arg(args, argc, argv);
        if(args->command){
            command = parse_command(args->command, strlen(args->command));
            call_command(command);
            free(command);
            return 0;
        }
    }
    char *buffer = calloc(4096, sizeof(char));
    while(1){
        write(STDOUT_FILENO, SH_PROMPT, strlen(SH_PROMPT));
        ssize_t n = 0;
        n += read(STDIN_FILENO, buffer, 4096);
        if(n == 0){
            puts("exit");
            return 0;
        }
        else if(n == 1){
            continue;
        }
        buffer[n-1] = '\0';
        command = parse_command(buffer, n);
        call_command(command);
        free(command);
    }
    return 0;
}
