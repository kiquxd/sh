#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>

size_t parse(char* arr, char** dst) {
    const char* home = getenv("HOME");
    size_t osz = 0;
    char* token = strtok(arr, " ");
    while (token != NULL) {
        // printf("%s %ld\n", token, strlen(token));
        size_t alloc_size = strlen(token);
        bool has_home = false;
        if (token[0] == '~') {
            alloc_size += strlen(home) - 1;
            has_home = true;
        }
        dst[osz] = (char*)malloc(alloc_size + 1);
        // printf("allocated\n");
        size_t offset = 0;
        if (has_home) {
            memcpy(dst[osz], home, strlen(home));
            offset = strlen(home);
        }
        memcpy(dst[osz] + offset, token + (offset == 0 ? 0 : 1), strlen(token) - (offset == 0 ? 0 : 1));
        // printf("copied\n");
        dst[osz++][alloc_size] = '\0';
        token = strtok(NULL, " ");
    }
    dst[osz] = NULL;
    return osz;
}

#define PIPE_FORK_ERR -1

int run(char** tokens, size_t size) {
    // simple shell without |, >/>>
    ssize_t pipe_idx = -1;
    for (size_t i = 0; i < size; ++i) {
        if (!strcmp(tokens[i], "|") && strlen(tokens[i]) == 1) {
            pipe_idx = i;
            break;
        }
    }
    if (pipe_idx != -1) {
        tokens[pipe_idx] = NULL;
        char** left = tokens;
        char** right = &tokens[pipe_idx + 1];
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe failed\n");
            return PIPE_FORK_ERR;
        }

        pid_t pid1 = fork();
        if (pid1 == -1) {
            return PIPE_FORK_ERR;
        }

        if (pid1 == 0) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);

            execvp(left[0], left);
            perror("execvp before pipe failed");
            exit(EXIT_FAILURE);
        }

        pid_t pid2 = fork();
        if (pid2 == -1) {
            return PIPE_FORK_ERR;
        }

        if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);

            int code = run(right, size - pipe_idx - 1);
            exit(code);
        }

        close(fd[0]);
        close(fd[1]);

        int status1, status2;
        waitpid(pid1, &status1, 0);
        waitpid(pid2, &status2, 0);

        if (WIFEXITED(status2)) {
            return WEXITSTATUS(status2);
        }
        return -1;
    }
    pid_t pid = fork();
    if (pid == -1) {
        return PIPE_FORK_ERR;
    }

    if (pid == 0) {
        for (size_t i = 0; i < size; ++i) {
            if (!strcmp(tokens[i], "|") && strlen(tokens[i]) == 1) {
                int fd[2];
                pipe(fd);
                pid_t pid2 = fork();
                if (pid2 == 0) {
                    tokens[i] = NULL;
                    dup2(STDIN_FILENO, fd[0]);
                    execvp(tokens[0], tokens);
                    perror("execvp failed");
                    exit(1);
                } else {
                    close(fd[0]);
                    dup2(fd[1], STDOUT_FILENO);
                    run(&tokens[i + 1], size - i);
                    close(fd[1]);
                }
            }
        }
        execvp(tokens[0], tokens);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return -1;
}

int main() {
    while (1) {
        printf("sh> ");
        fflush(stdout);

        char buf[1024];
        char* out_buf[64];

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }

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
        // printf("Tokens size: %ld\n", token_cnt);
        if (!strcmp(out_buf[0], "exit")) {
            return 0;
        }
        if (run(out_buf, token_cnt) != 0) {
            printf("Error occured\n");
        }
    }
}