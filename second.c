#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main(void) {
    char line[MAX_LINE];

    while (1) {
        printf("shell>>> ");

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (line[0] == '\0') {
            continue;
        }

        if (strcmp(line, "exit") == 0) {
            break;
        }

        char *argv[MAX_ARGS];
        int argc = 0;

        char *token = strtok(line, " ");
        while (token != NULL && argc < MAX_ARGS - 1) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            fprintf(stderr, "[child]  pid=%d  ppid=%d  cmd=%s\n",
                    (int)getpid(), (int)getppid(), argv[0]);

            execvp(argv[0], argv);
            perror("execvp");
            exit(127);
        } else {
            fprintf(stderr, "[parent] pid=%d created child=%d\n",
                    (int)getpid(), (int)pid);

            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                continue;
            }

            if (WIFEXITED(status)) {
                fprintf(stderr, "[parent] child=%d exited, code=%d\n",
                        (int)pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                fprintf(stderr, "[parent] child=%d killed by signal %d\n",
                        (int)pid, WTERMSIG(status));
            } else {
                fprintf(stderr, "[parent] child=%d ended (other status)\n", (int)pid);
            }
        }
    }
}
