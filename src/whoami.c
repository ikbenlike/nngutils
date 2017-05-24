#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char **argv){
    struct passwd *pw = getpwuid(geteuid());
    if(pw == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    puts(pw->pw_name);
    return 0;
}
