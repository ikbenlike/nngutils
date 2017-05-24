#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

int main(int argc, char **argv){
    bool silent = false;
    if(argc > 1){
        if(strcmp(argv[1], "--quiet")){
            silent = true;
        }
        else if(strcmp(argv[1], "-s")){
            silent = true;
        }
        else if(strcmp(argv[1], "--silent")){
            silent = true;
        }
        else {
            fprintf(stderr, "%s: incorrect argument %s\n", argv[0], argv[1]);
            return 1;
        }
    }
    char *tty = ttyname(STDIN_FILENO);
    if(tty == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    if(silent == false){
        puts(tty);
    }
    return 0;
}
