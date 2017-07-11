#include <stdio.h>
#include <string.h>
#include <utime.h>
#include <errno.h>

int main(int argc, char **argv){
    if(argc == 1){
        fprintf(stderr, "%s: missing file operand\n", argv[0]);
        return 1;
    }
    for(int i = 1; argv[i] != NULL; i++){
        if(utime(argv[i], NULL) == -1){
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            return 1;
        }
    }
    return 0;
}
