#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct tee_args {
    bool append;
    bool ignore_interrupts;
    bool diagnose;
    char **files;
    int fcount;
};

struct tee_args *parse_arg(struct tee_args *args, int argc, char **argv){
    args->files = calloc(1, sizeof(char*) * argc);
    args->fcount = 0;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            if(!strcmp(argv[i], "-a")){
                args->append = true;
            }
            else if(!strcmp(argv[i], "-i")){
                args->ignore_interrupts = true;
            }
            else if(!strcmp(argv[i], "-p")){
                args->diagnose = true;
            }
            else {
                fprintf(stderr, "%s: invalid argument '%s'\n", argv[0], argv[i]);
                exit(1);
            }
        }
        else {
            args->files[args->fcount++] = argv[i];
        }
    }
    return args;
}

int main(int argc, char **argv){
    struct tee_args *args = calloc(1, sizeof(struct tee_args));
    args = parse_arg(args, argc, argv);
    int *fds = calloc(1, sizeof(int) * args->fcount);
    for(int i = 0; i < args->fcount; i++){
        if(!strcmp(args->files[i], "-")){
            fds[i] = STDIN_FILENO;
            continue;
        }
        fds[i] = open(args->files[i], O_WRONLY);
        if(fds[i] == -1){
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            return 1;
        }
    }
    char *buf = calloc(1, 4096 * sizeof(char));
    if(buf == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    int buf_len = read(STDIN_FILENO, buf, 4096);
    while(buf_len != 0){
        for(int i = 0; i < args->fcount; i++){
            write(fds[i], buf, buf_len);
        }
        write(STDOUT_FILENO, buf, buf_len);
        buf_len = read(STDIN_FILENO, buf, 4096);
    }
    return 0;
}
