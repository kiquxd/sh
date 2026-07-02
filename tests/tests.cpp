#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <regex>
#include <iostream>
#include <stdexcept>

struct ProcessResult {
    std::string stdout_data;
    std::string stderr_data;
    int exit_code;
};

ProcessResult run_cmd(const std::string& bin, const std::vector<std::string>& args, const std::string& input) {
    int pipe_stdin[2], pipe_stdout[2], pipe_stderr[2];
    if (pipe(pipe_stdin) == -1 || pipe(pipe_stdout) == -1 || pipe(pipe_stderr) == -1) {
        throw std::runtime_error("Failed to create pipes");
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Failed to fork");
    }

    if (pid == 0) {
        dup2(pipe_stdin[0], STDIN_FILENO);
        dup2(pipe_stdout[1], STDOUT_FILENO);
        dup2(pipe_stderr[1], STDERR_FILENO);

        close(pipe_stdin[0]); close(pipe_stdin[1]);
        close(pipe_stdout[0]); close(pipe_stdout[1]);
        close(pipe_stderr[0]); close(pipe_stderr[1]);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(bin.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(bin.c_str(), argv.data());
        perror("execvp failed");
        exit(127);
    }

    close(pipe_stdin[0]);
    close(pipe_stdout[1]);
    close(pipe_stderr[1]);

    size_t written = 0;
    while (written < input.size()) {
        ssize_t res = write(pipe_stdin[1], input.data() + written, input.size() - written);
        if (res <= 0) break;
        written += res;
    }
    close(pipe_stdin[1]);

    std::string out;
    char buf[4096];
    ssize_t bytes;
    while ((bytes = read(pipe_stdout[0], buf, sizeof(buf))) > 0) {
        out.append(buf, bytes);
    }
    close(pipe_stdout[0]);

    std::string err;
    while ((bytes = read(pipe_stderr[0], buf, sizeof(buf))) > 0) {
        err.append(buf, bytes);
    }
    close(pipe_stderr[0]);

    int status;
    waitpid(pid, &status, 0);

    ProcessResult res;
    res.stdout_data = out;
    res.stderr_data = err;
    res.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return res;
}

std::string clean_shell_output(const std::string& output) {
    std::regex prompt_regex(R"(sh \(in [^\)]+\)> )");
    return std::regex_replace(output, prompt_regex, "");
}

TEST_CASE("Shell Integration Tests", "[shell]") {
    std::string my_shell = MYSHELL_PATH;

    SECTION("Test 1: Simple execution") {
        auto res_myshell = run_cmd(my_shell, {}, "echo Hello World\nexit\n");
        auto res_bash = run_cmd("bash", {"--noprofile", "--norc"}, "echo Hello World\nexit\n");

        REQUIRE(clean_shell_output(res_myshell.stdout_data) == res_bash.stdout_data);
    }

    SECTION("Test 2: Standard Pipe") {
        auto res_myshell = run_cmd(my_shell, {}, "echo hello world | tr a-z A-Z\nexit\n");
        auto res_bash = run_cmd("bash", {"--noprofile", "--norc"}, "echo hello world | tr a-z A-Z\nexit\n");

        REQUIRE(clean_shell_output(res_myshell.stdout_data) == res_bash.stdout_data);
    }

    SECTION("Test 3: Multi-stage Pipe") {
        auto res_myshell = run_cmd(my_shell, {}, "printf \"apple\\nbanana\\ncherry\\n\" | grep an | tr a-z A-Z\nexit\n");
        auto res_bash = run_cmd("bash", {"--noprofile", "--norc"}, "printf \"apple\\nbanana\\ncherry\\n\" | grep an | tr a-z A-Z\nexit\n");

        REQUIRE(clean_shell_output(res_myshell.stdout_data) == res_bash.stdout_data);
    }

    SECTION("Test 4: Navigation (cd)") {
        auto res_myshell = run_cmd(my_shell, {}, "cd /tmp\npwd\nexit\n");
        auto res_bash = run_cmd("bash", {"--noprofile", "--norc"}, "cd /tmp\npwd\nexit\n");

        REQUIRE(clean_shell_output(res_myshell.stdout_data) == res_bash.stdout_data);
    }

    SECTION("Test 5: Home expansion (cd ~)") {
        auto res_myshell = run_cmd(my_shell, {}, "cd ~\npwd\nexit\n");
        auto res_bash = run_cmd("bash", {"--noprofile", "--norc"}, "cd ~\npwd\nexit\n");

        REQUIRE(clean_shell_output(res_myshell.stdout_data) == res_bash.stdout_data);
    }

    SECTION("Test 6: Error Handling for Non-existent commands") {
        auto res_myshell = run_cmd(my_shell, {}, "nonexistentcommand123\nexit\n");
        
        REQUIRE_THAT(res_myshell.stdout_data, Catch::Matchers::ContainsSubstring("Error occurred"));
    }
}