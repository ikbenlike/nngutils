#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>

struct uname_args {
    bool p_all;
    bool p_kname;
    bool p_nname;
    bool p_krelease;
    bool p_kversion;
    bool p_machine;
    bool p_processor;
    bool p_platform;
    bool p_os;
    int count;
};


struct uname_args *parse_arg(struct uname_args *args ,int argc, char **argv){
    //struct uname_args args;
    args->count = 0;
    for(int i = 1; i < argc; i++){
        if(!strcmp("-a", argv[i]) || !strcmp("--all", argv[i])){
            args->p_all = true;
            args->count = 8;
            return args;
        }
        else if(!strcmp("-s", argv[i]) || !strcmp("--kernel-name", argv[i])){
            args->p_kname = true;
            args->count += 1;
        }
        else if(!strcmp("-n", argv[i]) || !strcmp("--nodename", argv[i])){
            args->p_nname = true;
            args->count += 1;
        }
        else if(!strcmp("-r", argv[i]) || !strcmp("--kernel-release", argv[i])){
            args->p_krelease = true;
            args->count += 1;
        }
        else if(!strcmp("-v", argv[i]) || !strcmp("--kernel-version", argv[i])){
            args->p_kversion = true;
            args->count += 1;
        }
        else if(!strcmp("-m", argv[i]) || !strcmp("--machine", argv[i])){
            args->p_machine = true;
            args->count += 1;
        }
        else if(!strcmp("-p", argv[i]) || !strcmp("--processor", argv[i])){
            args->p_processor = true;
            args->count += 1;
        }
        else if(!strcmp("-i", argv[i]) || !strcmp("--hardware-platform", argv[i])){
            args->p_platform = true;
            args->count += 1;
        }
        else if(!strcmp("-o", argv[i]) || !strcmp("--operating-system", argv[i])){
            args->p_os = true;
            args->count += 1;
        }
        else{
            fprintf(stderr, "%s: incorrect argument %s\n", argv[0], argv[i]);
            exit(1);
        }
    }
    return args;
}




int main(int argc, char **argv){
    struct uname_args *args = calloc(1, sizeof(struct uname_args));
    args = parse_arg(args, argc, argv);

    struct utsname *buf = calloc(1, sizeof(struct utsname));
    if(uname(buf) == -1){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        free(args);
        free(buf);
        return 1;
    }

    if(args->p_kname || args->p_all || argc == 1){
        fputs(buf->sysname, stdout);
        if(args->count > 1){
            putchar(' ');
        }
    }
    if(args->p_nname || args->p_all){
        fputs(buf->nodename, stdout);
        if(args->count > 1){
            putchar(' ');
        }
    }
    if(args->p_krelease || args->p_all){
        printf(buf->release, stdout);
        if(args->count > 1){
            putchar(' ');
        }
    }
    if(args->p_kversion || args->p_all){
        fputs(buf->version, stdout);
        if(args->count > 1){
            putchar(' ');
        }
    }
    if(args->p_machine || args->p_all){
        fputs(buf->machine, stdout);
        if(args->count > 1){
            putchar(' ');
        }
    }
    if(args->p_platform || args->p_processor || args->p_all){
        if(args->p_all == false){
            printf("unknown");
            if(args->count > 1){
                putchar(' ');
            }
        }
    }
    if(args->p_os || args->p_all){
        printf("GNU/Linux"); //just hardcoded for now
    }
    putchar('\n');
    free(args);
    free(buf);
    return 0;
}
