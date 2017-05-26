#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv){
    if(argc > 2){
        fprintf(stderr, "%s: extra operand '%s'\n", argv[0], argv[2]);
        return 1;
    }
    else if(argc < 2){
        fprintf(stderr, "%s: missing operand\n", argv[0]);
        return 1;
    }
    else {
        if(unlink(argv[1]) == -1){
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            return 1;
        }
    }
    return 0;
}
