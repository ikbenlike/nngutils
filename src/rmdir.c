#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct rmdir_args {
    bool ignore_fail;
    bool parents;
    bool verbose;
    char **paths;
    int pcount;
};

struct rmdir_args *parse_arg(struct rmdir_args *args, int argc, char **argv){
    args->paths = calloc(1, sizeof(char*) * argc);
    args->pcount = 0;
    for(int i = 0; i < argc; i++){
        if(!strcmp(argv[i], "--ignore-fail-on-non-empty")){
            args->ignore_fail = true;
        }
        else if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--parents")){
            args->parents = true;
        }
        else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")){
            args->verbose = true;
        }
        else if(argv[i][0] == '-'){
            fprintf(stderr, "%s: invalid operand '%s'\n", argv[0], argv[i]);
            exit(1);
        }
        else {
            args->paths[args->pcount++] = argv[i];
        }
    }
    return args;
}


int main(int argc, char **argv){
    if(argc == 1){
        fprintf(stderr, "%s: missing operand\n", argv[0]);
        return 1;
    }
    struct rmdir_args *args = calloc(1, sizeof(struct rmdir_args));
    args = parse_arg(args, argc, argv);
    for(int i = 1; i < args->pcount; i++){
        if(args->verbose){
            fprintf(stdout, "%s: removing directory, '%s'\n", argv[0], args->paths[i]);
        }
        if(rmdir(args->paths[i]) == -1){
            if(errno == ENOTEMPTY && !args->ignore_fail){
                fprintf(stderr, "%s: failed to remove '%s': %s\n", argv[0], args->paths[i], strerror(errno));
                return 1;
            }
        }
    }
    return 0;
}
