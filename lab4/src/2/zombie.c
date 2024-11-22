#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork error");
        return 1;
    } else if (pid == 0) {
        printf("Child process is exited\n");
        exit(0);
    } else {
        printf("Let's wait 10 sec to maintain the zombie process...\n");
        sleep(10); 
        printf("Parent process is exited.\n");
    }
    return 0;
}