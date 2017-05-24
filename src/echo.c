#include <stdio.h>


int main(int argc, char **argv){
    if(argc == 1){
        putchar('\n');
        return 0;
    }
    for(int i = 1; i < argc; i++){
        printf("%s ", argv[i]);
    }
    puts("");
    return 0;
}
