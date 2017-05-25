#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>


struct cat_args {
    bool show_all;
    bool number_nonblank;
    bool show_ends;
    bool number;
    bool squeeze_blank;
    bool show_tabs;
    bool show_nonprinting;
    char **files;
    int fcount;
};


struct cat_args *parse_arg(struct cat_args *args, int argc, char **argv){
    args->fcount = 0;
    args->files = calloc(1, argc * sizeof(char*));
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-' && strlen(argv[i]) != 1){
            if(!strcmp(argv[i], "-A") || !strcmp(argv[i], "--show-all")){
                args->show_all = true;
            }
            else if(!strcmp(argv[i], "-e")){
                args->show_nonprinting = true;
                args->show_ends = true;
            }
            else if(!strcmp(argv[i], "-n") || !strcmp(argv[i], "--number")){
                args->number = true;
            }
            else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--number-nonblank")){
                args->number_nonblank = true;
                args->number = false;
            }
            else if(!strcmp(argv[i], "-s") || !strcmp(argv[i], "--squeeze-blank")){
                args->squeeze_blank = true;
            }
            else if(!strcmp(argv[i], "-t")){
                args->show_nonprinting = true;
                args->show_tabs = true;
            }
            else if(!strcmp(argv[i], "-T") || !strcmp(argv[i], "--show-tabs")){
                args->show_tabs = true;
            }
            else if(!strcmp(argv[i], "-u")){
                continue;
            }
            else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--show-nonprinting")){
                args->show_nonprinting = true;
            }
            else {
                fprintf(stderr, "%s: incorrect argument %s\n", argv[0], argv[i]);
                exit(1);
            }
        }
        else {
            args->files[args->fcount++] = argv[i];
        }
    }
    return args;
}

int print_file(int fd, struct cat_args *args, char *name){
    int endchar;
    static unsigned long int nline = 0;
    char *buf = calloc(1, 4096 * sizeof(char));
    if(buf == NULL){
        fprintf(stderr, "%s: %s\n", name, strerror(errno));
        exit(1);
    }
    int buf_len = read(fd, buf, 4096);
    if(args->number){
        if(nline < 9){
            printf("     %lu  ", ++nline);
        }
        else if(nline < 99){
            printf("    %lu  ", ++nline);
        }
        else if(nline < 999){
            printf("   %lu  ", ++nline);
        }
        else if(nline < 9999){
            printf("  %lu  ", ++nline);
        }
        else if(nline < 99999){
            printf(" %lu  ", ++nline);
        }
        else{ 
            printf("%lu  ", ++nline);
        }
    }
    while(buf_len != 0){
        for(int i = 0; i < buf_len; i++){
            if(args->show_ends && buf[i] == '\n'){
                fputc('$', stdout);
            }
            else if(args->show_tabs && buf[i] == '\t'){
                fputs("^I", stdout);
                continue;
            }
            else if(args->number && buf[i] == '\n' && i + 1 < buf_len){
                fputc(buf[i], stdout);
                if(nline < 9){
                    printf("     %lu  ", ++nline);
                }
                else if(nline < 99){
                    printf("    %lu  ", ++nline);
                }
                else if(nline < 999){
                    printf("   %lu  ", ++nline);
                }
                else if(nline < 9999){
                    printf("  %lu  ", ++nline);
                }
                else if(nline < 99999){
                    printf(" %lu  ", ++nline);
                }
                else{ 
                    printf("%lu  ", ++nline);
                }
                continue;
            }
            fputc(buf[i], stdout);
        }
        buf_len = read(fd, buf, 4096);
    }
    free(buf);
    return 0;
}

int cat(struct cat_args *args, char *name){
    int fd;
    int c;
    for(int i = 0; i < args->fcount; i++){
        if(!strcmp(args->files[i], "-")){
            fd = STDIN_FILENO;
        }
        else {
            fd = open(args->files[i], O_RDONLY);
            if(fd == -1){
                fprintf(stderr, "%s: %s\n", name, strerror(errno));
                exit(1);
            }
        }
        print_file(fd, args, name);
        close(fd);
    }
    return 0;
}

int main(int argc, char **argv){
    struct cat_args *args = calloc(1, sizeof(struct cat_args));
    if(args == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    args = parse_arg(args, argc, argv);
    if(argc == 1 || args->fcount == 0){
        args->files[0] = calloc(1, 2 * sizeof(char));
        strcpy(args->files[0], "-");
        args->fcount = 1;
    }
    cat(args, argv[0]);
    free(args->files);
    free(args);
    return 0;
}
