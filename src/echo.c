#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

struct echo_args {
    bool nonewline;
    bool backslash;
    int count;
};

struct echo_args *parse_args(struct echo_args *args, int argc, char **argv){
    args->count = 0;
    if(argv[1][0] == '-'){    
        for(int i = 1; i < argc; i++){
            if(!strcmp(argv[i], "-n")){
                args->nonewline = true;
                args->count += 1;
            }
            else if(!strcmp(argv[i], "-e")){
                args->backslash = true;
                args->count += 1;
            }
            else if(!strcmp(argv[i], "-E")){
                args->backslash = false;
                args->count += 1;
            }
        }
    }
    return args;
}

int main(int argc, char **argv){
    struct echo_args *args = calloc(1, sizeof(struct echo_args));
    args = parse_args(args, argc, argv);
    if(argc == 1){
        putchar('\n');
        return 0;
    }
    for(int i = args->count + 1; i < argc; i++){
        printf("%s", argv[i]);
        if(i < argc - 1){
            putchar(' ');
        }
    }
    if(args->nonewline == false){
        putchar('\n');
    }
    return 0;
}
