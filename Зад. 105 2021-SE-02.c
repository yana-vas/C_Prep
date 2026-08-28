#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>

int main(int argc, char *argv[]) {

    if(argc != 3) {
        errx(1, "Invalid number of args");
    }

    int f_in = open(argv[1], O_RDONLY);
    if(f_in == -1) {
        err(1, "Cannot open input file");
    }

    int f_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(f_out == -1) {
        err(1, "Cannot open output file");
    }

    struct stat st;
    if(fstat(f_in, &st) == -1) {
        err(1, "Cannot fstat");
    }
    if(st.st_size % sizeof(uint16_t) != 0 ){
        errx(1, "Invalid size of input file");
    }

    uint8_t in_bytes[2];
    ssize_t read_bytes;
    while((read_bytes = read(f_in, &in_bytes, sizeof(uint16_t))) > 0) {
        uint16_t num = ((uint16_t)in_bytes[0] << 8) | in_bytes[1];

        uint8_t out_byte = 0;
        for (int i = 15; i >0; i-=2) {
            uint8_t bit1 = (num >> i) & 1;
            uint8_t bit2 = (num >> (i-1)) & 1;

            uint8_t decoded_bit;
            if( bit1 == 0 && bit2 == 1) {
                decoded_bit = 0;
            } else if (bit1 == 1 && bit2 == 0) {
                decoded_bit = 1;
            } else {
                errx(1, "Invalid Manchester enocoding");
            }

            out_byte = (out_byte << 1) | decoded_bit;
        }

        if(write(f_out, &out_byte, sizeof(uint8_t)) != sizeof(uint8_t)) {
            errx(1, "Cannot write to output file");
        }
    }
    if(read_bytes == -1) {
        err(1, "Error reading from input file");
    }

    close(f_in);
    close(f_out);

    return 0;
}
