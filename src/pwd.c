#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv){
    char pwd[PATH_MAX];

    if(!getcwd(pwd, sizeof(pwd))){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    puts(pwd);
    return 0;
}
