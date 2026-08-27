#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <stdio.h>

struct Header {
    uint16_t magic;
    uint16_t ver;
    uint16_t cp;
    uint16_t co;
};


struct Object {
    uint32_t ctime;
    uint16_t opt;
    uint16_t parent_id;
    uint32_t size;
    uint32_t ssize;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        errx(1, "Invalid number of arguments");
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        err(1, "Cannot open input file");
    }

    struct Header hdr;
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        err(1, "Cannot read header");
    }

    if (hdr.magic != 0x6963) {
        errx(1, "Invalid magic number");
    }
    if (hdr.ver != 0x6e73) {
        errx(1, "Invalid file version");
    }

    if (lseek(fd, hdr.cp * 8, SEEK_CUR) == -1) {
        err(1, "Cannot lseek past preamble");
    }


    struct Object objects[hdr.co];
    ssize_t expected_bytes = (ssize_t)hdr.co * sizeof(struct Object);
    if (read(fd, objects, expected_bytes) != expected_bytes) {
        err(1, "Cannot read objects array");
    }

    uint64_t total_ssize = 0;
    uint64_t total_size = 0;

    for(uint16_t i = 0; i < hdr.co; i++) {
        if((objects[i].opt >> 14) == 2 && objects[i].parent_id != 0) {
            struct Object parent = objects[objects[i].parent_id];
            uint32_t diff_time = objects[i].c_time - parent.c_time;
            if(diff_time >= 85800 && diff_time <=87000 ) {
                total_size+=objects[i].size;
                total_ssize+=objects[i].ssize;
            }
        }
    }

    if(total_size > 0) {
        double avg = (double) total_ssize / total_size;
        char out[64];
        int len = snprintf(out, sizeof(out), "%f\n", avg);
        if(write(1, &out, len) == -1) {
            err(1, "Cannot output");
        }
    }


    close(fd);
