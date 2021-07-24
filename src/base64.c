#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>

struct base64_args {
    bool decode;
    bool ignore_garbage;
    long wrap;
    bool help;
    bool version;
    char *file;
};

void handle_error(int e, struct base64_args *args){
    fprintf(stderr, "Error: %s\n", strerror(e));
    free(args->file);
    free(args);
    exit(e);
}

struct base64_args *parse_args(struct base64_args *args, int argc, char **argv){
    args->wrap = 76;
    for(size_t i = 1; i < argc; i++){
        if(!strcmp("-d", argv[i]) || !strcmp("--decode", argv[i])){
            args->decode = true;
        }
        else if(!strcmp("-i", argv[i]) || !strcmp("--ignore-garbage", argv[i])){
            args->ignore_garbage = true;
        }
        else if(!strcmp("-w", argv[i])){
            if(i + 1 < argc){
                args->wrap = strtol(argv[++i], NULL, 10);
                if(args->wrap == 0 && errno == EINVAL){
                    fputs("Missing amount of columns for -w flag!", stderr);
                    exit(1);
                }
            }

        }
        else if(!strncmp("--wrap=", argv[i], 7)){
            args->wrap = strtol(argv[i]+7, NULL, 10);
            if(args->wrap == 0 && errno == EINVAL){
                fputs("Missing amount of columns in --wrap= flag!", stderr);
                exit(1);
            }
        }
        else if(!strcmp("--help", argv[i])){
            args->help = true;
        }
        else if(!strcmp("--version", argv[i])){
            args->version = true;
        }
        else {
            if(args->file == NULL){
                args->file = strdup(argv[i]);
            }
            else {
                fprintf(stderr, "Extra argument %s", argv[i]);
                exit(1);
            }
        }
    }
}

#define TABLE_SIZE 64

const char *table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char lookup(char c){
    assert(c < TABLE_SIZE);
    return table[c];
}

char *tripletoquad(char *x, char *r){

    r[0] = lookup(x[0] >> 2);
    r[1] = lookup(((x[0] & 0b00000011) << 4) | ((x[1] & 0b11110000) >> 4));
    r[2] = lookup(((x[1] & 0b00001111) << 2) | ((x[2] & 0b11000000) >> 6));
    r[3] = lookup(x[2] & 0b00111111);

    return r;
}

void wrap_quad(char *quad, struct base64_args *args){
    static int printed = 0;
    for(int i = 0; i < 4; i++){
        if(printed++ == args->wrap && args->wrap != 0){
            putchar('\n');
            printed = 1;
        }
        putchar(quad[i]);
    }
}

void encode_buffer(ssize_t size, char *buf, struct base64_args *args){
    //assert(buf[2] == '\n');
    size_t last_triplet = size - size % 3;
    size_t i = 0;
    char *quad = calloc(4, sizeof(char));
    for(; i < last_triplet; i += 3){
        tripletoquad(&buf[i], quad);
        wrap_quad(quad, args);
        memset(quad, 0, 4);
    }
    if(i < size){
        char *triplet = calloc(3, sizeof(char));
        size_t diff = size - i;
        triplet[0] = buf[i];
        if(diff == 2){
            triplet[1] = buf[i+1];
            tripletoquad(triplet, quad);
            quad[3] = '=';
        }
        else if(diff == 3 || diff == 1){
            tripletoquad(triplet, quad);
            quad[2] = '=';
            quad[3] = '=';
        }
        wrap_quad(quad, args);
        if(args->wrap != 0){
            putchar('\n');
        }
    }
    free(quad);
}

void encode_file(struct base64_args *args){
    char *buf = calloc(4096, sizeof(char));
    int fd = open(args->file, O_RDONLY);
    if(fd == -1){
        handle_error(errno, args);
    }

    ssize_t r = 0;
    for(r = read(fd, buf, 4095); r > 0; r = read(fd, buf, 4095)){
        encode_buffer(r, buf, args);
    }

    free(buf);
    close(fd);

    if(r == -1){
        handle_error(errno, args);
    }
}

int main(int argc, char **argv){
    struct base64_args *args = calloc(1, sizeof(struct base64_args));
    parse_args(args, argc, argv);

    /*printf("decode: %d\n", args->decode);
    printf("ignore garbage: %d\n", args->ignore_garbage);
    printf("wrap: %d\n", args->wrap);
    printf("help: %d\n", args->help);
    printf("version: %d\n", args->version);
    printf("file: %s\n", args->file);*/

    if(args->file != NULL && strcmp(args->file, "-") && args->decode == false){
        encode_file(args);
    }

    return 0;
}
