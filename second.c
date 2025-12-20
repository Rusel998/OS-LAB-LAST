#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_JOBS 128

typedef struct {
    int job_id;
    pid_t pid;
    char cmdline[MAX_LINE];
    int active;
} Job;

static Job jobs[MAX_JOBS];
static int next_job_id = 1;

static volatile sig_atomic_t child_exited = 0;

static void sigchld_handler(int signo) {
    (void)signo;
    child_exited = 1;
}

static void reap_children(void) {
    int status;
    pid_t p;

    while ((p = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].active && jobs[i].pid == p) {
                jobs[i].active = 0;

                if (WIFEXITED(status)) {
                    printf("\n[job %d] pid=%d finished, exit=%d: %s\n",
                           jobs[i].job_id, (int)p, WEXITSTATUS(status), jobs[i].cmdline);
                } else if (WIFSIGNALED(status)) {
                    printf("\n[job %d] pid=%d killed by signal %d: %s\n",
                           jobs[i].job_id, (int)p, WTERMSIG(status), jobs[i].cmdline);
                } else {
                    printf("\n[job %d] pid=%d finished: %s\n",
                           jobs[i].job_id, (int)p, jobs[i].cmdline);
                }
                fflush(stdout);
                break;
            }
        }
    }
}

static void add_job(pid_t pid, const char *cmdline, int job_id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            jobs[i].active = 1;
            jobs[i].pid = pid;
            jobs[i].job_id = job_id;
            snprintf(jobs[i].cmdline, sizeof(jobs[i].cmdline), "%s", cmdline);
            return;
        }
    }
    fprintf(stderr, "Too many background jobs!\n");
}

int main(void) {
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = 0;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    char line[MAX_LINE];

    while (1) {
        
        if (child_exited) {
            child_exited = 0;
            reap_children();
        }

        printf("shell[%d]>>> ", next_job_id);
        fflush(stdout);

        errno = 0;
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (errno == EINTR) {
                continue;
            }
            putchar('\n');
            break;
        }

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

        if (line[0] == '\0') continue;
        if (strcmp(line, "exit") == 0) break;

        int background = 0;
        {
            size_t l = strlen(line);
            while (l > 0 && line[l - 1] == ' ') {
                line[l - 1] = '\0'; l--;
            }
            if (l > 0 && line[l - 1] == '&') {
                background = 1;
                line[l - 1] = '\0';
                l--;
                while (l > 0 && line[l - 1] == ' ') { line[l - 1] = '\0'; l--; }
            }
        }

        char cmdline_copy[MAX_LINE];
        snprintf(cmdline_copy, sizeof(cmdline_copy), "%s%s", line, background ? " &" : "");

        char *argv[MAX_ARGS];
        int argc = 0;

        char *token = strtok(line, " ");
        while (token != NULL && argc < MAX_ARGS - 1) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        if (argv[0] == NULL) {
            continue;
        }

        int job_id = next_job_id++;
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp");
            _exit(127);
        } else {
            if (background) {
                add_job(pid, cmdline_copy, job_id);
                printf("[job %d] started pid=%d: %s\n", job_id, (int)pid, cmdline_copy);
            } else {
                int status;
                if (waitpid(pid, &status, 0) == -1) {
                    perror("waitpid");
                }
            }
        }
    }
}
