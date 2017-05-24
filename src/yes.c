#include <stdio.h>

int main(int argc, char **argv){
    if(argc == 1){
        while(1){
            puts("y");
        }
    }
    else {
        while(1){
            for(int i = 1; i < argc; i++){
                printf("%s ", argv[i]);
            }
            putchar('\n');
        }
    }
    return 0;
}
