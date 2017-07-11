#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct tac_args {
    bool before;
    bool regex;
    bool separator;
    char *sep;
    char **files;
    int nfiles;
};

struct tac_args *parse_arg(struct tac_args *args, int argc, char **argv){
    args->files = calloc(1, sizeof(char*) * argc);
    args->nfiles = 0;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            if(!strcmp(argv[i], "-b")){
                args->before = true;
            }
            else if(!strcmp(argv[i], "-r")){
                args->regex = true;
            }
            else if(!strcmp(argv[i], "-s")){
                args->separator = true;
                if(i + 1 < argc){
                    args->sep = argv[++i];
                }
                else {
                    fprintf(stderr, "%s: missing operand after '-s'\n");
                    exit(1);
                }
           }
        }
        else {

        }
    }
}

int main(int argc, char **argv){
    
}
