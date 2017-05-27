#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

extern char **environ;

struct env_args {
    bool ignore_env;
    bool null;
    char **unset;
    int nunset;
    char *command;
    char **args;
    int argc;
};

struct env_args *parse_arg(struct env_args *args, int argc, char **argv){
    args->nunset = 0;
    args->unset = calloc(1, sizeof(char*) * argc);
    args->args = calloc(1, sizeof(char*) * argc);
    args->argc = 0;
    args->command = NULL;
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--ignore-environment") || !strcmp(argv[i], "-")){
            args->ignore_env = true;
        }
        else if(!strcmp(argv[i], "-0") || !strcmp(argv[i], "--null")){
            args->null = true;
        }
        else if(!strcmp(argv[i], "-u")){
            if(i + 1 >= argc){
                fprintf(stderr, "%s: missing operand after -u\n", argv[0]);
                exit(1);
            }
            else {
                args->unset[args->nunset++] = argv[++i];
            }
        }
        else {
            args->command = argv[i++];
            args->args[args->argc++] = args->command;
            for(int a = i; a < argc; a++){
                args->args[args->argc++] = argv[a];
            }
            args->args[args->argc] = NULL;
            break;
        }
    }
    return args;
}

int main(int argc, char **argv){
    if(argc == 1){
        for(int i = 0; environ[i] != NULL; i++){
            puts(environ[i]);
        }
        return 0;
    }
    struct env_args *args = calloc(1, sizeof(struct env_args));
    args = parse_arg(args, argc, argv);
    if(args->ignore_env){
        *environ = NULL;
    }
    if(args->nunset > 0){
        for(int i = 0; i < args->nunset; i++){
            puts(args->unset[i]);
        }
    }
    if(args->nunset > 0){
        for(int i = 0; i < args->nunset; i++){
            if(unsetenv(args->unset[i]) == -1){
                fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
                return 1;
            }
        }
    }
    if(args->command == NULL){
        for(int i = 0; environ[i] != NULL; i++){
            puts(environ[i]);
        }
        return 0;
    }
    if(execvp(args->command, args->args) == -1){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    return 0;
}
