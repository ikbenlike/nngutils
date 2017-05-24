#include <stdio.h>

int main(int argc, char **argv){
    if(argc == 1){
        while(1){
            putchar('y');
        }
    }
    else {
        while(1){
            for(int i = 1; i < argc; i++){
                printf("%s ", argv[i]);
            }
        }
    }
    return 0;
}
