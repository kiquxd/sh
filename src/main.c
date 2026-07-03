#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>

void free_tokens(char** tokens, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (tokens[i] != NULL) {
            free(tokens[i]);
            tokens[i] = NULL;
        }
    }
}

const size_t MAX_TOKENS = 64;

size_t parse(char* prompt, char** dst) {
    const char* home = getenv("HOME");
    if (home == NULL) {
        home = "";
    }
    size_t osz = 0;
    char* token = strtok(prompt, " \t\r\n");
    while (token != NULL && osz < MAX_TOKENS - 1) {
        size_t alloc_size = strlen(token);
        bool has_home = false;
        
        if (token[0] == '~' && (token[1] == '\0' || token[1] == '/')) {
            alloc_size += strlen(home) - 1;
            has_home = true;
        }
        
        dst[osz] = (char*)malloc(alloc_size + 1);
        if (dst[osz] == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        
        size_t offset = 0;
        if (has_home) {
            memcpy(dst[osz], home, strlen(home));
            offset = strlen(home);
        }
        
        memcpy(dst[osz] + offset, token + (has_home ? 1 : 0), strlen(token) - (has_home ? 1 : 0));
        dst[osz++][alloc_size] = '\0';
        token = strtok(NULL, " \t\r\n");
    }
    dst[osz] = NULL;
    return osz;
}

#define PIPE_FORK_ERR -1

int run(char** tokens, size_t size) {
    if (size == 0 || tokens == NULL || tokens[0] == NULL) {
        return 0;
    }

    ssize_t pipe_idx = -1;
    for (size_t i = 0; i < size; ++i) {
        if (tokens[i] != NULL && strcmp(tokens[i], "|") == 0) {
            pipe_idx = i;
            break;
        }
    }
    
    if (pipe_idx != -1) {
        free(tokens[pipe_idx]);
        tokens[pipe_idx] = NULL;
        
        char** left = tokens;
        char** right = &tokens[pipe_idx + 1];
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe failed");
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
        char cwd[1024];
        printf("sh (in %s)> ", getcwd(cwd, sizeof(cwd)));
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
        
        const char* exit_cmd = "exit";
        if (strlen(out_buf[0]) == strlen(exit_cmd) && strncmp(out_buf[0], exit_cmd, strlen(exit_cmd) + 1) == 0) {
            free_tokens(out_buf, token_cnt);
            return 0;
        }

        const char* cd_cmd = "cd";
        if (strlen(out_buf[0]) == strlen(cd_cmd) && strncmp(out_buf[0], cd_cmd, strlen(cd_cmd) + 1) == 0) {
            char* target_dir = NULL;
            if (token_cnt == 1) {
                target_dir = getenv("HOME");
                if (target_dir == NULL) {
                    fprintf(stderr, "cd: HOME not set\n");
                    free_tokens(out_buf, token_cnt);
                    continue;
                }
            } else if (token_cnt == 2) {
                target_dir = out_buf[1];
            } else {
                fprintf(stderr, "cd: too many arguments\n");
            }

            if (target_dir != NULL) {
                if (chdir(target_dir) != 0) {
                    perror("cd failed");
                }
            }
            free_tokens(out_buf, token_cnt);
            continue;
        }
        
        if (run(out_buf, token_cnt) != 0) {
            printf("Error occurred\n");
        }
        free_tokens(out_buf, token_cnt);
    }
    return 0;
}
