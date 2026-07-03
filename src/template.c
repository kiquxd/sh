#include <stdio.h>

const size_t MAX_TOKENS = 64;

size_t parse(char* arr, char** dst) {
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

        if (strcmp(out_buf[0], "exit") == 0) {
            // TODO
            return 0;
        }
        
        if (strcmp(out_buf[0], "cd") == 0) {
            // TODO
            continue;
        }
        
        // TODO
    }
    return 0;
}
