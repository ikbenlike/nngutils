#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv, char **environ){
    for(int i = 0; environ[i] != NULL; i++){
        puts(environ[i]);
    }
    return 0;
}
