#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if(argc != 4) {
        errx(1, "Invlaid number of arguments");
    }

    int f_scl = open(argv[1], O_RDONLY);
    if(f_scl == -1) {
        err(1, "Cannot open SCL file");
    }

    int f_sdl = open(argv[2], O_RDONLY);
    if(f_sdl == -1) {
        err(1, "Cannot open SDL file");
    }

    int f_out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(f_out == -1) {
        err(1, "Cannot open the output file");
    }

    struct stat st;
    if(fstat(f_sdl, &st) == -1) {
        err(1, "Cannot fstat");
    }
    if(st.st_size % sizeof(uint16_t) != 0) {
        errx(1, "Invalid size of SDL file");
    }

    struct stat st_scl;
    if(fstat(f_scl, &st_scl) == -1) {
        err(1, "Cannot fstat");
    }
    if(st_scl.st_size * 16 != st.st_size) {
        errx(1, "Inconsistent file sizes between SCL and SDL");
    }


    uint8_t scl_byte;
    ssize_t read_bytes;
    while((read_bytes = read(f_scl, &scl_byte, sizeof(uint8_t))) > 0) {

        for(int i = 7; i >= 0; i--) {
            uint8_t curr_bit = ( scl_byte >> i) & 1;

            uint16_t num;
            if(read(f_sdl, &num, sizeof(uint16_t)) == -1) {
                err(1, "Cannot read from SDL file");
            }

            if(curr_bit == 1) {
                if(write(f_out, &num, sizeof(uint16_t)) == -1) {
                    err(1, "Cannot write to output file");
                }
            }
        }
    }

    if(read_bytes == -1) {
        err(1, "Error while reading from SDL file");
    }

    close(f_scl);
    close(f_sdl);
    close(f_out);

    return 0;
}
