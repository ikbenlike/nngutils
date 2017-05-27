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
    char **vars;
    int nvar;
};

struct env_args *parse_arg(struct env_args *args, int argc, char **argv){
    args->unset = calloc(1, sizeof(char*) * argc);
    args->args = calloc(1, sizeof(char*) * argc);
    args->vars = calloc(1, sizeof(char*) * argc);
    args->nunset = 0;
    args->nvar = 0;
    args->argc = 0;
    args->command = NULL;
    bool com = false;
    if(args->unset == NULL || args->args == NULL || args->vars == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        exit(1);
    }
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
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
        }
        else if(strchr(argv[i], '=') != NULL && com == false){
            args->vars[args->nvar++] = argv[i];
        }
        else {
            args->command = argv[i];
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
    if(args == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    args = parse_arg(args, argc, argv);
    if(args->ignore_env){
        *environ = NULL;
    }
    if(args->nunset > 0){
        for(int i = 0; i < args->nunset; i++){
            puts(args->unset[i]);
        }
    }
    if(args->nvar > 0){
        for(int i = 0; i < args->nvar; i++){
            if(putenv(args->vars[i]) != 0){
                fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
                return 1;
            }
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
        free(args->unset);
        free(args->vars);
        free(args->args);
        free(args);
        return 0;
    }
    if(execvp(args->command, args->args) == -1){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    free(args->unset);
    free(args->vars);
    free(args->args);
    free(args);
    return 0;
}
