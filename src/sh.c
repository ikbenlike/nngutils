#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

extern char **environ;

struct sh_args {
    bool interactive;
    bool standard_input;
    bool has_command;
    char *command;
};

struct sh_args *parse_arg(struct sh_args *args, int argc, char **argv){
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            if(!strcmp(argv[i], "-c")){
                args->has_command = true;
                if(i + 1 < argc){
                    args->command = argv[++i];
                }
                else {
                    fprintf(stderr, "%s: -c option requires an argument\n", argv[i]);
                }
            }
            else if(!strcmp(argv[i], "-s")){
                args->standard_input = true;
            }
            else if(!strcmp(argv[i], "-i")){
                args->interactive = true;
            }
        }
        else {
            fprintf(stderr, "%s: invalid argument %s\n", argv[0], argv[i]);
        }
    }
    return args;
}

int call_command(char **argv){
    int status = 0;
    pid_t pid = fork();
    if(pid == -1){
        fprintf(stderr, "sh: %s\n", strerror(errno));
        return 1;
    }
    else if(pid){
        waitpid(pid, &status, 0);
    }
    else {
        exit(execvp(argv[0], argv));
    }
    return status;
}

int main(int argc, char **argv){
    struct sh_args *args = calloc(1, sizeof(struct sh_args));
    if(args == NULL){
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
    }
    args = parse_arg(args, argc, argv);
    if(args->command){
        puts(args->command);
        return call_command(&args->command);
    }
    char buf[4096];
    while(read(STDIN_FILENO, buf, 4096)){
        call_command((char**)buf);
    }
    return 0;
}
