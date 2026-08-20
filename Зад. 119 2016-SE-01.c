#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>

int main(int argc, char *argv[]) {

    if(argc != 2) {
        errx(1, "Invalid number or ags");
    }

    int pfd[2];
    if ( pipe(pfd) == -1 ) {
        err(1, "Cannot pipe");
    }

    pid_t pid = fork();
    if( pid == -1 ) {
        err(1, "fork failed");
    }

    if (pid == 0) {
        dup2(pfd[1], 1);
        close(pfd[0]);
        close(pfd[1]);
        execlp("cat", "cat", argv[1], (char *)NULL);

        err(1,"exec cat failed");

        exit(0);
    }

    pid_t pid2 = fork();
    if(pid2 == -1) {
        err(1, "fork failed");
    }

    if (pid2 == 0){
        dup2(pfd[0], 0);
        close(pfd[0]);
        close(pfd[1]);
        execlp("sort", "sort", (char *)NULL);

        err(1, "exec sort failed");
        exit(0);
    }

    close(pfd[0]);
    close(pfd[1]);
    wait(NULL);
    wait(NULL);

    return 0;
}
