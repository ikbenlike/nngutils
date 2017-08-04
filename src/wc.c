#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>


struct wc_args {
    bool byte_count;
    bool char_count;
    bool line_count;
    bool max_len;
    bool word_count;
    char **files;
    int fcount;
};

struct wc_args *parse_arg(struct wc_args *args, int argc, char **argv){
    args->fcount = 0;
    args->files = calloc(1, argc * sizeof(char*));
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-' && strlen(argv[i]) != 1){
            if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--bytes")){
                args->byte_count = true;
            }
            else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--chars")){
                args->char_count = true;
            }
            else if(!strcmp(argv[i], "-l") || !strcmp(argv[i], "--lines")){
                args->line_count = true;
            }
            else if(!strcmp(argv[i], "-L") || !strcmp(argv[i], "--max-line-length")){
                args->max_len = true;
            }
            else if(!strcmp(argv[i], "-w") || !strcmp(argv[i], "--words")){
                args->word_count = true;
            }
            else {
                fprintf(stderr, "%s: incorrect argument %s\n", argv[0], argv[i]);
            }
        }
        else {
            args->files[args->fcount++] = argv[i];
        }
    }
    if(args->fcount == 0){
        args->files[args->fcount++] = "-";
    }
    if(args->fcount == argc - 1 || args->fcount == argc){
        args->line_count = true;
        args->word_count = true;
        args->byte_count = true;
    }
    return args;
}

typedef struct {
    off_t size;
    ssize_t lines;
    ssize_t words;
} wc_data;

off_t wc_get_byte_count(int fd){
    struct stat s;
    if(fstat(fd, &s) < 0){
        fprintf(stderr, "wc: %s\n", strerror(errno));
        exit(1);
    }
    return s.st_size;
}

/*wc_data wc_parse_file(int fd){
    char buf[4096];
    size_t linec = 0;
    size_t wordc = 0;
    ssize_t n;
    struct stat s;
    if(fstat(fd, &s) < 0){
        fprintf(stderr, "wc: %s\n", strerror(errno));
        exit(1);
    }
    while((n = read(fd, buf, 4096)) > 0){
        for(size_t i = 0; i < n; i++){
            if(buf[i] == '\n'){
                linec++;
            }
            else if(i >= 1){
                if(!isspace(buf[i-1]) && isspace(buf[i])){
                    wordc++;
                }
            }
        }

    }
    wc_data d;
    d.size = s.st_size;
    d.lines = linec;
    d.words = wordc;
    return d;
}

void formatted_print(ssize_t wordc, ssize_t linec, ssize_t bytec, char *name){
    if(wordc == -1 && linec == -1 && bytec != -1){
        printf("%zu %s\n", (size_t)bytec, name);
    }
    else if(bytec == -1 && wordc == -1 && linec != -1){
        printf("%zu %s\n", (size_t)bytec, name);
    }
    else if(linec == -1 && bytec == -1 && wordc != -1){
        printf("%zu %s\n", (size_t)bytec, name);
    }
    else if(wordc == -1 && linec != -1 && bytec != -1){
        printf("%zu %zu %s\n", (size_t)linec, (size_t)bytec, name);
    }
    else if(bytec == -1 && wordc != -1 && linec != -1){
        printf("%zu %zu %s\n", (size_t)wordc, (size_t)linec);
    }
}*/

size_t wc_get_nl_count(int fd){
    char buf[4096];
    size_t sum = 0;
    ssize_t n;
    lseek(fd, 0, SEEK_SET);
    while((n = read(fd, buf, 4095)) > 0){
        for(size_t i = 0; i < n; i++){
            if(buf[i] == '\n'){
                sum++;
            }
        }
    }
    return sum;
}

size_t wc_get_word_count(int fd){
    char buf[4096];
    size_t num = 0;
    ssize_t n;
    lseek(fd, 0, SEEK_SET);
    while((n = read(fd, buf, 4096)) > 0){
        for(size_t i = 1; i < n; i++){
            if(!isspace(buf[i-1]) && isspace(buf[i])){
                num++;
            }
        }
    }
    return num;
}

int main(int argc, char **argv){
    struct wc_args *args = calloc(1, sizeof(struct wc_args));
    if(args == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        return 1;
    }
    args = parse_arg(args, argc, argv);
    for(int i = 0; i < args->fcount; i++){
        int fd = open(args->files[i], O_RDONLY);
        if(!strcmp(args->files[i], "-")){
            fd = STDIN_FILENO;
        }
        if(fd < 0){
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            return 1;
        }
        /*if(args->byte_count){
            puts("byte count");
            printf("%zu\n", wc_get_byte_count(fd));
        }
        if(args->line_count){
            puts("line count");
            printf("%zu\n", wc_get_nl_count(fd));
        }
        if(args->word_count){
            puts("word count");
            printf("%zu\n", wc_get_word_count(fd));
        }*/
        printf("  %zu %zu %zu %s\n", wc_get_nl_count(fd), wc_get_word_count(fd), wc_get_byte_count(fd), args->files[i]);
    }
    return 0;
}
