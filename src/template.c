#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>

void free_tokens(char** tokens, size_t size) {
    return;
}

const size_t MAX_TOKENS = 64;

char* expand_single_token(const char* token) {
    return token;
}

void expand_env(char** tokens, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (tokens[i] == NULL) {
            continue;
        }

        char* expanded = expand_single_token(tokens[i]);
        free(tokens[i]);
        tokens[i] = expanded;
    }
}


size_t parse(char* prompt, char** dst) {
    return 0;
}

bool handle_builtin(char** tokens, size_t size, int* status) {
    const char* exit_cmd = "exit";
    if (strncmp(tokens[0], exit_cmd, strlen(exit_cmd) + 1) == 0) {
        return false;
    }
    
    const char* cd_cmd = "cd";
    if (strncmp(tokens[0], cd_cmd, strlen(cd_cmd) + 1) == 0) {
        return false;
    }

    const char* export_cmd = "export";
    
    if (strncmp(tokens[0], export_cmd, strlen(export_cmd) + 1) == 0) {
        return false;
    }
    
    return false;
}

#define PIPE_FORK_ERR -1

int run(char** tokens, size_t size) {
    return 0;
}

int main() {
    while (1) {
        char cwd[1024];
        printf("sh (in %s)> ", cwd);
        fflush(stdout);

        char buf[1024];
        char* out_buf[64];

        // Handle input

        size_t len = strlen(buf);
        if (len == 0) {
            continue;
        }
        if (buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
            --len;
        }
        size_t token_cnt = parse(buf, out_buf);
        if (token_cnt == 0 || out_buf[0] == NULL) {
            continue;
        }

        expand_env(out_buf, token_cnt);
        
        if (run(out_buf, token_cnt) != 0) {
            fprintf(stderr, "Error occurred\n");
        }
        free_tokens(out_buf, token_cnt);
    }
    return 0;
}
