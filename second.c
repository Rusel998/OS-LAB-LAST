#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE 1024     // максимальная длина вводимой строки
#define MAX_ARGS 64      // максимальное количество аргументов

int main(void) {
    char line[MAX_LINE];

    while (1) {
        printf("shell>>> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (line[0] == '\0') {
            continue;
        }

        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
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

        if (argc == 0) {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(argv[0], argv);

            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
            }
        }
    }

    printf("Выход из интерпретатора команд.\n");
    return 0;
}

