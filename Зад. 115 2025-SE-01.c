#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>

struct Header {
    uint64_t id;
    uint8_t N;
    char buff[256];
};

struct F_Info {
    int fd;
    char role[256];
    uint64_t curr_time;
    char buff[256];
    bool active;
};

int main (int argc, char *argv[]) {

    if(argc < 2 || argc > 21) {
        errx(1, "Invalid args");
    }

    bool AreAnyActive = false;
    struct F_Info open_files[20];
    for(int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if(fd == -1) {
            err(1, "Cannot open the %d file", i);
        }

        uint64_t id;
        uint8_t h_N;
        if(read(fd, &id, sizeof(uint64_t)) != sizeof(uint64_t)) {
            err(1, "Cannot read element from the %d file", i);
        }

        if(id != 133742) {
            errx(1, "Invalid header id");
        }
        if(read(fd, &h_N, sizeof(uint8_t)) != sizeof(uint8_t)) {
            err(1, "read");
        }
        struct F_Info fi;
        if(read(fd, fi.role, h_N) != h_N) {
            err(1, "read");
        }
        fi.role[h_N] = '\0';

        uint64_t curr_time;
        if(read(fd, &curr_time, sizeof(uint64_t)) != sizeof(uint64_t)) {
            err(1, "read");
        }
        uint8_t N;
        if(read(fd, &N, sizeof(uint8_t)) != sizeof(uint8_t)) {
            err(1, "read");
        }
        char buff[N];
        if(read(fd, fi.buff, N) != N) {
            err(1, "read");
        }
        fi.buff[N] = '\0';
        fi.fd = fd;
        fi.curr_time = curr_time;
        fi.active = true;
        open_files[i-1] = fi;
        AreAnyActive = true;


    }

    while (AreAnyActive) {
        AreAnyActive = false;

        int minIdx = 0;
        uint64_t minTime = UINT64_MAX;

        for(int i = 0; i < argc-1; i++) {
            if(open_files[i].active == false) {
                continue;
            }
            AreAnyActive = true;
            if(open_files[i].curr_time < minTime) {
                minTime = open_files[i].curr_time;
                minIdx = i;
            }
        }
        if(!AreAnyActive) {
            break;
        }
        char buff[600];
        int len = snprintf(buff, sizeof(buff), "%s: %s\n", open_files[minIdx].role, open_files[minIdx].buff);
        write(1, buff, len);
        uint64_t new_time;
        int len_r = read(open_files[minIdx].fd, &new_time, sizeof(uint64_t));
        if(len_r == 0) {
            open_files[minIdx].active = false;
            continue;
        } else if(len_r == -1) {
            err(1, "read");
        }

        uint8_t new_N;
        if(read(open_files[minIdx].fd, &new_N, sizeof(uint8_t)) != sizeof(uint8_t)) {
            err(1, "read");
        }

        if(read(open_files[minIdx].fd, open_files[minIdx].buff, new_N) != new_N) {
            err(1, "read");
        }
        open_files[minIdx].buff[new_N] = '\0';

        open_files[minIdx].curr_time = new_time;
    }

    for(int i = 1; i < argc; i++){
        close(argv[i]);
    }

    return 0;
}
