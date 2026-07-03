#include <stdio.h>
#include <string.h>

const size_t MAX_TOKENS = 64;

void free_tokens(char** tokens, size_t size) {
    return;
}

size_t parse(char* prompt, char** dst) {
    return 0;
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

        const char* exit_cmd = "exit";
        if (strlen(out_buf[0]) == strlen(exit_cmd) && strncmp(out_buf[0], exit_cmd, strlen(exit_cmd) + 1) == 0) {
            // TODO
            return 0;
        }
        
        const char* cd_cmd = "cd";
        if (strlen(out_buf[0]) == strlen(cd_cmd) && strncmp(out_buf[0], cd_cmd, strlen(cd_cmd) + 1) == 0) {
            // TODO
            continue;
        }
        
        // TODO
    }
    return 0;
}
