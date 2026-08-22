#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char *argv[]) {

    if (argc != 2) {
        errx(1, "Invalid number of args");
    }

    int pfd[2];
    if(pipe(pfd) == -1){
        err(1, "failed pipe");
    }

    pid_t pid = fork();
    if(pid == -1) {
        err(1, "fork failed");
    }

    if(pid == 0){
        dup2(pfd[1], 1);
        close(pfd[0]);
        close(pfd[1]);

        if (execlp("tar", "tar", "-cf", "-", argv[1], (char *)NULL) == -1) {
            err(1, "Could not execute tar");
        }

        exit(0);
    }

    close(pfd[1]);

    uint8_t hash = 0;
    uint8_t buff[4096];

    ssize_t read_bytes;
    while((read_bytes = read(pfd[0], buff, sizeof(buff))) > 0) {

        for(int i = 0; i < read_bytes; i++){
            hash ^= buff[i];
        }
    }

    close(pfd[0]);

    wait(NULL);


    if(read_bytes == -1) {
        err(1, "Error reading from output of tar");
    }

    char out[64];
    int len = snprintf(out, sizeof(out), "%.2X\n", hash);
    if(write(1, out, len) == -1){
        err(1, "Error while writing to the output");
    }

    return 0;
}
