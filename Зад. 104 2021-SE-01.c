#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>

int main(int argc, char *argv[]) {

    if ( argc != 3) {
        errx(1, "Invalid number of args");
    }

    int in = open(argv[1], O_RDONLY);
    if (in == -1) {
        err(1, "Cannot open finput file");
    }

    int out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(out == -1) {
        err(1, "Cannot open output file");
    }

    struct stat st;
    if(fstat(in, &st) == -1) {
        err(1, "Cannot stat input file");
    }
    if(st.st_size % sizeof(uint8_t) != 0) {
        errx(1, "Size not correct of input file");
    }

    uint8_t in_byte;
    ssize_t read_bytes;
    while((read_bytes = read(in, &in_byte, sizeof(in_byte))) > 0) {
        uint8_t b_out[2] = {0, 0};
        for(int i=7; i>=0; i--) {
            int bit = (in_byte >> i) & 1;
            uint8_t val;
            if (bit == 1) {
                val=2;
            } else if(bit == 0) {
                val=1;
            }
            if(i < 4) {
                b_out[1] = (b_out[1] << 2 | val);
            } else if (i>=4) {
                b_out[0] = (b_out[0] << 2 | val);
            }
        }

            if(write(out, b_out, sizeof(b_out)) == -1 ) {
                err(1, "Cannot write to output file");
            }

    }

    if (read_bytes == -1) {
        err(1, "Error reading from input file");
    }

    close(in);
    close(out);

    return 0;
}
