#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

void info(const char *name) {
    struct timeval tv;
    gettimeofday(&tv, 0);

    struct tm t = *localtime(&tv.tv_sec);

    printf("%s: pid=%d ppid=%d time=%02d:%02d:%02d:%03ld\n",
           name,
           getpid(),
           getppid(),
           t.tm_hour,
           t.tm_min,
           t.tm_sec,
           tv.tv_usec / 1000);
}

int main() {
    pid_t p1 = fork();

    if (p1 == 0) {
        info("CHILD 1");
        return 0;
    }

    pid_t p2 = fork();

    if (p2 == 0) {
        info("CHILD 2");
        return 0;
    }

    info("PARENT");
    system("ps -x");
}
