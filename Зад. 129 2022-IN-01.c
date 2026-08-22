#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if(argc != 3) {
        errx(1, "Invalid number or args");
    }

    int N = atoi(argv[1]);
    int D = atoi(argv[2]);

    int p2c[2];
    int c2p[2];

    if(pipe(p2c) == -1) {
        err(1, "Failed pipe");
    }
    if(pipe(c2p) == -1) {
        err(1, "Failed pipe");
    }

    pid_t pid1=fork();

    if(pid1 == -1) {
        err(1, "fork failed");
    }

    char dummy;
    if(pid1 == 0){
        close(p2c[1]);
        close(c2p[0]);

        for(int i = 0; i < N; i++) {
            if (read(p2c[0], &dummy, 1) == -1) {
                err(1, "Failed reading to p2c (waiting for parent)");
            }
            if(write(1, "DONG\n", 5) == -1) {
                err(1, "Failed to output \"DONG\"");
            }
            if(write(c2p[1], &dummy, 1) == -1){
                err(1, "Failed writing to c2p");
            }
        }

        close(p2c[0]);
        close(c2p[1]);

        exit(0);
    }

    close(p2c[0]);
    close(c2p[1]);

    for(int i = 0; i<N; i++) {
        if(write(1, "DING ", 5) == -1) {
            err(1, "Failed to output \"DING\"");
        }
        if(write(p2c[1], &dummy, 1) == -1) {
            err(1, "Failed writing to p2c (sending to child)");
        }
        if(read(c2p[0], &dummy, 1) == -1) {
            err(1, "Failed reading from c2p");
        }
        sleep(D);
    }

    close(p2c[1]);
    close(c2p[0]);
    wait(NULL);
    exit(0);

    return 0;
}
