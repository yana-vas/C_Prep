#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>


int main(int argc, char *argv[]) {

    if(argc < 2) {
        exit(26);
    }

    int pfd[2];
    if(pipe(pfd) == -1) {
        exit(26);
    }

    pid_t pids[argc -1];
    for(int i = 1; i < argc; i++) {
        pid_t pid = fork();
        if(pid == -1) {
            exit(26);
        }
        if(pid == 0) {
            if(dup2(pfd[1], 1) == -1) {
                exit(26);
            }
            close(pfd[0]);
            close(pfd[1]);
            execlp(argv[i], argv[i], (char *)NULL);
            exit(26);
        }
        pids[i-1] = pid;
    }
    close(pfd[1]);

    const char word[10] = "found it!";
    char c;
    int i = 0;
    bool found = false;
    ssize_t rb;
    while((rb = read(pfd[0], &c, sizeof(char))) > 0) {
        if(c == word[i]) {
            i++;
            if(word[i] == '\0') {
                found = true;
                break;
            }
        } else {
            i = 0;
            if(c == word[0]) {
                i = 1;
            }
        }
    }
    if(rb == -1) {
        exit(26);
    }

    close(pfd[0]);
    if(found) {
        for(int i = 0; i < argc -1; i++) {
            kill(pids[i], SIGTERM);
        }

        for(int i = 0; i < argc -1; i++) {
            wait(NULL);
        }

        exit(0);
    } else {
        for(int i = 0; i < argc -1; i++) {
            wait(NULL);
        }
        exit(1);
    }



    return 0;
}
