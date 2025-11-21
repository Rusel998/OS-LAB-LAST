#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>

void print_info(const char *label) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *tm_info = localtime(&tv.tv_sec);

    int hours   = tm_info->tm_hour;
    int minutes = tm_info->tm_min;
    int seconds = tm_info->tm_sec;
    long millis = tv.tv_usec / 1000;

    printf("[%s] pid=%d, ppid=%d, time=%02d:%02d:%02d:%03ld\n",
           label,
           getpid(),
           getppid(),
           hours, minutes, seconds, millis);
}

int main(void) {
    pid_t pid1, pid2;

    printf("Старт программы (пока один процесс)\n");
    print_info("START");

    pid1 = fork();

    if (pid1 < 0) {
        perror("fork1");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        print_info("CHILD 1");
        sleep(5);
        return 0;
    }

    pid2 = fork();

    if (pid2 < 0) {
        perror("fork2");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        print_info("CHILD 2");
        sleep(5);
        return 0;
    }

    print_info("PARENT (before ps)");

    printf("\n--- Выполнение команды ps -x (родительский процесс) ---\n\n");
    system("ps -x");

    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);

    print_info("PARENT (after children finished)");
    printf("Родительский процесс завершён.\n");

    return 0;
}

