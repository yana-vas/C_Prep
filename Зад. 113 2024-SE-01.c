#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

struct Entry {
    uint64_t next;
    char user_data[504];
};

int main(int argc, char *argv[]) {

    if(argc != 2) {
        errx(1, "Invlaid number of args");
    }

    int fd = open(argv[1], O_RDWR);
    if(fd == -1) {
        err(1, "Cannot open file");
    }

    struct stat st;
    if(fstat(fd, &st) == -1) {
        err(1, "Cannot fstat");
    }
    if(st.st_size % sizeof(struct Entry) != 0) {
        errx(1, "Invalid file size");
    }

    char temp_name[] = "/tmp/visited_XXXXXX";
    int tmp_fd = mkstemp(temp_name);
    unlink(temp_name);

    uint64_t curr_node = 0;

    struct Entry en;
    while (true) {

        if(lseek(tmp_fd, curr_node, SEEK_SET) == -1) {
            err(1, "Cannot lseek");
        }

        uint8_t m = 1;
        if(write(tmp_fd, &m, sizeof(uint8_t)) == -1) {
            err(1, "Cannot write to tmp file");
        }

        if(lseek(fd, curr_node * 512, SEEK_SET) == -1) {
            err(1, "Cannot lseek");
        }

        if(read(fd, &en, sizeof(struct Entry)) == -1) {
            err(1, "Cannot read from fd");
        }

        curr_node = en.next;

        if(curr_node == 0) {
            break;
        }
    }

    uint64_t total_nodes = st.st_size / sizeof(struct Entry);
    uint8_t zero_node[512] = {0};
    for(uint64_t i = 0; i < total_nodes; i++) {
        if (lseek(tmp_fd, i, SEEK_SET) == -1) {
            err(1, "Cannot lseek");
        }
        uint8_t tmp_val = 0;
        if(read(tmp_fd, &tmp_val, sizeof(uint8_t)) == -1) {
            err(1, "Cannot read");
        }
        if(tmp_val != 1) {
            if(lseek(fd, i * 512, SEEK_SET) == -1) {
                err(1, "Cannot lseek");
            }
            if(write(fd, &zero_node, sizeof(zero_node)) == -1) {
                err(1, "Cannot write to file");
            }
        }
    }

    close(fd);
    close(tmp_fd);
    return 0;
}
