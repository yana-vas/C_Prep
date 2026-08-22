#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>
#include <stdio.h>


struct Entry {
    char name[8];
    uint32_t offset;
    uint32_t length;
};

int main(int argc, char *argv[]) {

    if(argc != 2) {
        errx(1, "Invalid number of args");
    }

    int f = open(argv[1], O_RDONLY);
    if(f == -1) {
        err(1, "Cannot open file");
    }

    struct stat st;
    if(fstat(f, &st) == -1) {
        err(1, "Cannot fstat");
    }

    if(st.st_size % sizeof(struct Entry) != 0) {
        errx(1, "Invalid size of the input file");
    }

    int pfd[2];
    if(pipe(pfd) == -1) {
        err(1, "Pipe failed");
    }

    int num_children=0;
    ssize_t read_bytes;

    struct Entry en;
    while((read_bytes = read(f, &en, sizeof(struct Entry))) > 0) {
        num_children++;
        pid_t pid = fork();
        if (pid == -1) {
            err(1, "fork failed");
        }

        if(pid == 0) {
            close(pfd[0]);

            int curr_f = open(en.name, O_RDONLY);
            if(curr_f == -1) {
                err(1, "Cannot open the file %s from the input file", en.name);
            }

            if(lseek(curr_f, en.offset*sizeof(uint16_t), SEEK_SET) == -1) {
                err(1, "lseek failed");
            }

            uint16_t res = 0;
            for (int i = 0; i < en.length; i++) {
                uint16_t curr_num;
                if(read(curr_f, &curr_num, sizeof(uint16_t)) == -1) {
                    err(1, "failed reading from the file %s", en.name);
                }

                res ^= curr_num;
            }

            if(write(pfd[1], &res, sizeof(res)) == -1) {
                err(1, "Cannot write to pipe");
            }

            close(curr_f);
            close(pfd[1]);
            exit(0);

        }

    }

    close(pfd[1]);

    uint16_t child_res;
    uint16_t final_res=0;
    while(read(pfd[0], &child_res, sizeof(uint16_t)) > 0){
        final_res ^= child_res;
    }

    close(pfd[0]);

    for(int i =0; i < num_children; i++) {
        wait(NULL);
    }

    char out_buff[64];
    int len = snprintf(out_buff, sizeof(out_buff), "result: %.4X\n", final_res);
    write(1, out_buff, len);

    if(read_bytes == -1) {
        err(1, "Error reading from the input file");
    }

    close(f);
    return 0;
}
