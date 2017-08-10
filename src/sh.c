#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

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

struct sh_command *parse_command(char *buffer){
    struct sh_command *command = calloc(1, sizeof(struct sh_command));
    command->argc = command_get_argc(buffer) + 1;
    command->argv = calloc(command->argc, sizeof(char*));
    size_t len = strlen(buffer);
    char *token = calloc(len + 1, sizeof(char));
    for(int i = 0, n = 0; i < command->argc && n < len; i++ && n++){
        int x = 0;
        while(buffer[n] != ' ' && buffer[n] != EOF && buffer[n] != '\0'){
            token[x++] = buffer[n]; 
        }
        token[x++] = '\0';
        command->argv[i] = calloc(x, sizeof(char));
        memcpy(command->argv[i], token, x);
    }
    return command;
}

int call_command(char **argv){
    puts("call command");
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
        puts("else");
        if(execvp(argv[0], argv) == -1){
            puts(argv[0]);
            fprintf(stderr, "sh: %s\n", strerror(errno));
            exit(1);
        }
    }
    return status;
}

int main(int argc, char **argv){
    struct sh_args *args = calloc(1, sizeof(struct sh_args));
    if(args == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
    }
    args = parse_arg(args, argc, argv);
    if(args->command){
        puts(args->command);
        return call_command(&args->command);
    }
    char **arg = calloc(2, sizeof(char*));
    arg[0] = calloc(4096, sizeof(char));
    arg[1] = NULL;
    while(1){
        int n = 0;
        while(n = read(STDIN_FILENO, arg[0], 4096)){
            printf("n = %d\n", n);
            arg[0][n-1] = '\0';
            printf("%d\n", call_command(arg));
        }
    }
    return 0;
}
