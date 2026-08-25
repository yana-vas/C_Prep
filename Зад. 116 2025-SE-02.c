#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    if(argc != 4) {
        errx(1, "Invalid number of args");
    }

    int f_idx = open(argv[1], O_RDONLY);
    if(f_idx == -1) {
        err(1, "Cannot open file idx");
    }

    int f_val = open(argv[2], O_RDONLY);
    if(f_val == -1) {
        err(1, "Cannot open file val");
    }

    char *val_name = argv[3];

    uint8_t byte;

    ssize_t read_bytes;
    uint32_t i = 0;

    uint8_t found = 0;
    while((read_bytes = read(f_idx, &byte, sizeof(uint8_t))) > 0) {
        uint8_t N = byte & 0x7F;
        uint8_t type = (byte >> 7) & 1;

        char name[128];
        if(read(f_idx, name, N) == -1) {
            err(1, "Error reading from file idx");
        }

        name[N]='\0';

        if (strcmp(name, val_name) == 0) {
            found=1;
            if(lseek(f_val, i * sizeof(uint32_t), SEEK_SET) == -1) {
                err(1, "Cannot lseek the val file");
            }

            int len;
            char out_buff[64];

            if (type == 0) {
                int32_t val;
                if(read(f_val, &val, sizeof(int32_t)) == -1) {
                    err(1, "Cannot read from val file");
                }

                len=snprintf(out_buff, sizeof(out_buff), "%d\n", val);
            }
            else if (type == 1) {
                float val;
                if(read(f_val, &val, sizeof(float)) == -1) {
                    err(1, "Cannot read from val file");
                }
                len= snprintf(out_buff, sizeof(out_buff), "%.3f\n", val);
            }

            if(write(1, out_buff, len) == -1) {
                err(1, "Cannot output");
            }
            break;
        }
        i++;
    }

    if(found == 0) {
        errx(1, "Record not found");
    }
    close(f_idx);
    close(f_val);
    if(read_bytes == -1) {
        err(1, "Error reading from the index file");
    }

    return 0;
}
