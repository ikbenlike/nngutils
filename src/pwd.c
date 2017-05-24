#include <unistd.h>
#include <limits.h>
#include <stdio.h>

int main(int argc, char **argv){
    char pwd[PATH_MAX];

    getcwd(pwd, sizeof(pwd));
    puts(pwd);
    return 0;
}
