#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv, char **environ){
    char *var;
    int ret = 0;
    if(argc == 1){
        for(int i = 0; environ[i] != NULL; i++){
            puts(environ[i]);
        }
    }
    else if(!strcmp(argv[1], "-0") || !strcmp(argv[1], "--null")){
        for(int i = 2; i < argc; i++){
            var = getenv(argv[i]);
            if(var == NULL){
                ret = 1;
                continue;
            }
            else {
                fprintf(stdout, "%s", var);
                putchar('\0');
            }
        }
    }
    else {
        for(int i = 1; i < argc; i++){
            var = getenv(argv[i]);
            if(var == NULL){
                ret = 1;
                continue;
            }
            else {
                fprintf(stdout, "%s\n", var);
            }
        }
    }
    return ret;
}
