#include <stdint.h>
#include <unistd.h>
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

    if ( argc != 3 ) {
        errx(1, "Invalid number of args");
    }

    int f1 = open(argv[1], O_RDONLY);
    if ( f1 == -1 ) {
        err(1, "Cannot open the first file");
    }

    int f2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (f2 == -1){
        err(1, "Cannot open the second file");
    }

    uint8_t msg[256];
    ssize_t read_bytes;

    uint8_t curr;
    while((read_bytes = read(f1, &curr, sizeof(curr))) > 0) {
        if ( curr == 0x55 ) {
            msg[0] = curr;

            if (read(f1, &msg[1], sizeof(uint8_t)) == -1) {
                err(1, "Error reading from the first file");
            }
            if (msg[1] < 3) {
                continue;
            }
            if (read(f1, &msg[2], msg[1]-2) == -1) {
                err(1, "Error reading from first file");
            }
            uint8_t calc_checksum = 0;

            for(int i = 0; i <= msg[1]-2; i++) {
                calc_checksum ^= msg[i];
            }

            if (calc_checksum == msg[msg[1] -1]) {
                if(write(f2, msg, msg[1]) == -1) {
                    err(1, "Error writing to the second file");
                }
            }


        }

    }

    if(read_bytes == -1) {
        err(1, "Error reading from the frist file");
    }

    close(f1);
    close(f2);

    return 0;
}
