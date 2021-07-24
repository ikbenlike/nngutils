#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define SH_PROMPT "nngsh-1.0$ "

enum sh_builtins {
    shb_exit,
    shb_alias,
    shb_unalias,
    shb_cd,
};

extern char **environ;

struct sh_alias {
    size_t id;
    char *name;
    char *command;
};

struct sh_args {
    bool interactive;
    bool standard_input;
    bool has_command;
    char *command;
};

struct sh_command {
    int out;
    int in;
    char **argv;
    int argc;
};




typedef struct {
    size_t len;
    size_t cursor;
    struct sh_alias *list;
} hashmap_vector_t;

typedef struct {
    size_t size;
    hashmap_vector_t *map;
} hashmap_t;

size_t hashmap_standalone_hash(char *inp){
    size_t output = 0;
    size_t i = 0;
    size_t l = strlen(inp);
    while((i + 1) <= l){
        output = output*17 + inp[i++];
    }
    return output;
}

size_t hashmap_hash(hashmap_t *hm, char *inp){
    size_t output = 0;
    size_t i = 0;
    size_t l = strlen(inp);
    while((i + 1) <= l){
        output = output*17 + inp[i++];
    }
    return output % hm->size;
}

hashmap_t *hashmap_init(size_t size){
    hashmap_t *hm = calloc(1, sizeof(hashmap_t));
    hm->map = calloc(size, sizeof(hashmap_vector_t));
    hm->size = size;
    for(size_t i = 0; i < hm->size; i++){
        hm->map[i].len = 3;
        hm->map[i].cursor = 0;
        hm->map[i].list = calloc(1, sizeof(struct sh_alias));
    }
    return hm;
}

int hashmap_free(hashmap_t *hm){
    for(int i = 0; i < hm->size; i++){
        free(hm->map[i].list);
    }
    free(hm->map);
    free(hm);
    return 0;
}

int hashmap_set(hashmap_t *hm, char *name, char *command){
    size_t c = hashmap_standalone_hash(name);
    size_t n = hashmap_hash(hm, name);
    hm->map[n].list[hm->map[n].cursor].id = c;
    hm->map[n].list[hm->map[n].cursor].name = name;
    hm->map[n].list[hm->map[n].cursor++].command = command;
    return 0;
}

ssize_t hashmap_item_exists(hashmap_t *hm, char *inp){
    size_t c = hashmap_standalone_hash(inp);
    size_t n = hashmap_hash(hm, inp);
    hashmap_vector_t a = hm->map[n];
    for(size_t i = 0; i < a.len; i++){
        if(a.list[i].id == c){
            return i;
        }
    }
    return -1;
}

struct sh_alias *hashmap_get(hashmap_t *hm, char *name){
    if(hashmap_item_exists(hm, name) != -1){
        size_t id = hashmap_standalone_hash(name);
        hashmap_vector_t v = hm->map[hashmap_hash(hm, name)];
        for(size_t i = 0; i < v.len; i++){
            if(v.list[i].id == id){
                return &v.list[i];
            }
        }
    }
    return NULL;
}

hashmap_t *alias_hashmap;





void sigint_handler(int sig){
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, SH_PROMPT, strlen(SH_PROMPT));
    return;
}

void sh_exit(int code){
    puts("exit");
    exit(code);
}

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

int command_get_argc(char *buffer){
    int n = 0;
    char *p;
    while(p = strchr(buffer, ' ')){
        n++;
        buffer = p + 1;
    }
    return n;
}

struct sh_command *parse_command(char *buffer, size_t len){
    struct sh_command *command = calloc(1, sizeof(struct sh_command));
    command->argc = command_get_argc(buffer) + 1;
    command->argv = calloc(command->argc + 1, sizeof(char*));
    char *token = calloc(len + 2, sizeof(char));
    for(int i = 0, n = 0; i < command->argc && n < len; i++ && n++){
        int x = 0;
        while(buffer[n] != ' ' && buffer[n] != EOF && buffer[n] != '\0'){
            token[x++] = buffer[n++];
        }
        if(i < 1){
            n++;
        }
        token[x++] = '\0';
        command->argv[i] = calloc(x, sizeof(char));
        memcpy(command->argv[i], token, x);
    }
    command->argv[command->argc] = NULL;
    free(token);
    return command;
}

int sh_is_builtin(char *command){
    if(!strcmp(command, "exit")){
        return shb_exit;
    }
    else if(!strcmp(command, "cd")){
        return shb_cd;
    }
    else if(!strcmp(command, "alias")){
        puts("alias");
        return shb_alias;
    }
    else if(!strcmp(command, "unalias")){
        return shb_unalias;
    }
    return -1;
}

int sh_add_alias(char *arg){
    size_t n_len = 0;
    size_t c_len = 0;
    char *name = NULL;
    char *command = NULL;
    
    char *split = strchr(arg, '=');

    if(split){
        n_len = split - arg;
        name = calloc(n_len + 1, sizeof(char));
        strncpy(name, arg, n_len);
        name[n_len] = '\0';

        c_len = strlen(arg) - n_len;
        command = calloc(c_len + 1, sizeof(char));
        strncpy(command, arg + n_len + 1, c_len);
    }


    printf("alias invoked with '%s' and '%s'\n", name, command);

    char *p_name = NULL;

    if(name[0] == '\'' && name[n_len - 1] == '\''){
        p_name = calloc(n_len - 1, sizeof(char));
        strncpy(p_name, name + 1, n_len);
        p_name[n_len - 1] = '\0';
    }
    else if(name[0] == '\'' && name[n_len - 1] != '\''){
        p_name = calloc(n_len, sizeof(char));
        strncpy(p_name, name + 1, n_len);
    }
    else if(name[0] != '\'' && name[n_len - 1] == '\''){
        p_name = calloc(n_len, sizeof(char));
        strncpy(p_name, name, n_len - 1);
        p_name[n_len - 1] = '\0';
    }

    char *p_command = NULL;

    if(command[0] == '\'' && name[c_len - 1] == '\''){
        p_command = calloc(c_len - 1, sizeof(char));
        strncpy(p_command, command, c_len);
        p_command[c_len - 1] = '\0';
    }
    else if(command[0] == '\'' && name[n_len - 1] != '\''){
        p_command = calloc(c_len, sizeof(char));
        strncpy(p_command, name + 1, c_len);
    }

    printf("aliased '%s' to '%s'\n", p_name, p_command);

    return hashmap_set(alias_hashmap, p_name, p_command);

}

int sh_cd(char *path){
    if(!path){
        char *res = getenv("HOME");
        if(res){
            path = res;
        }
        else {
            return 0;
        }
    }
    int cd_res = chdir(path);
    if(cd_res == -1){
        fprintf(stderr, "cd: %s: %s\n", strerror(errno), path);
        return errno;
    }
    return 0;
}

int run_builtin(enum sh_builtins code, struct sh_command *command){
    if(code == shb_exit){
        if(command->argc == 2){
            char **end;
            int ret = (int)strtol(command->argv[1], end, 10);
            if(end[0][0] == command->argv[1][0]){
                fprintf(stderr, "sh: exit: %s: numeric argument required\n", command->argv[1]);
                sh_exit(2);
            }
            sh_exit(ret);
        }
        sh_exit(0);
    }
    else if(code == shb_cd){
        return sh_cd(command->argv[1]);
    }
    else if(code == shb_alias){
        if(command->argc == 2){
            return sh_add_alias(command->argv[1]);
        }
        else {
            puts("else");
            return 1;
        }
    }
    return 0;
}

int call_command(struct sh_command *command){
    int status = 0;
    int builtin = sh_is_builtin(command->argv[0]);
    if(builtin != -1){
        status = run_builtin(builtin, command);
        return status;
    }
    pid_t pid = fork();
    if(pid == -1){
        fprintf(stderr, "sh: %s\n", strerror(errno));
        return 1;
    }
    else if(pid){
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)){
            status = WEXITSTATUS(status);
        }
    }
    else {
        if(execvp(command->argv[0], command->argv) == -1){
            fprintf(stderr, "sh: %s: %s\n", command->argv[0], strerror(errno));
            for(int i = -1; command->argv[++i] != NULL; free(command->argv[i]));
            free(command->argv);
            free(command);
            exit(1);
        }
    }
    return status;
}

int main(int argc, char **argv){
    signal(SIGINT, sigint_handler);
    alias_hashmap = hashmap_init(100);
    struct sh_command *command = NULL;
    if(argc > 1){
        struct sh_args *args = calloc(1, sizeof(struct sh_args));
        if(args == NULL){
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        }
        args = parse_arg(args, argc, argv);
        if(args->command){
            command = parse_command(args->command, strlen(args->command));
            call_command(command);
            free(command);
            return 0;
        }
    }
    char *buffer = calloc(4096, sizeof(char));
    while(1){
        write(STDOUT_FILENO, SH_PROMPT, strlen(SH_PROMPT));
        ssize_t n = 0;
        n += read(STDIN_FILENO, buffer, 4096);
        if(n == 0){
            sh_exit(0);
        }
        else if(n == 1){
            continue;
        }
        buffer[n-1] = '\0';
        command = parse_command(buffer, n);
        call_command(command);
        free(command);
    }
    return 0;
}
