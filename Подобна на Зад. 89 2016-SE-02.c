#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <err.h>

struct Command {
    uint32_t offset;
    uint16_t count;
    uint16_t min_val;
};

int main(int argc, char *argv[]) {

    if ( argc != 4 ) {
        errx(1, "Error: Args should be exactly 4");
    }

    int commands = open(argv[1], O_RDONLY);
    if ( commands == -1 ) {
        err(1, "Cannot open commands file");
    }

    int data = open(argv[2], O_RDONLY);
    if ( data == -1 ) {
        err(1, "Cannot open data file");
    }

    int out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if ( out == -1 ) {
        err(1, "Cannot open out file");
    }

    struct stat st;
    if (fstat(commands, &st) == -1) {
        err(1, "Cannot stat commands file");
    }

    if (st.st_size % sizeof(struct Command) != 0 ) {
        errx(1, "Invalid file format of command file (size must be a multiple of %lu)", sizeof(struct Command));
    }

    struct stat st2;
    if (fstat(data, &st2) == -1 ){
        err(1, "Cannot stat data file");
    }

    if ( st2.st_size % sizeof(uint16_t) != 0 ) {
        errx(1, "Invalid file format");
    }

    struct Command command;
    ssize_t read_bytes;
    while ( (read_bytes = read(commands, &command, sizeof(struct Command))) > 0 ) {
        if (lseek(data, command.offset * sizeof(uint16_t), SEEK_SET) == -1 ) {
            err(1, "Cannot lseek in commands file");
        }

        for (uint32_t i = 0; i < command.count; i++) {
            uint16_t num;

            if(read(data, &num, sizeof(num)) == -1 ) {
                err(1, "Cannot read element from data file");
            }

            if ( num >= command.min_val ) {
                if(write(out, &num, sizeof(num)) == -1 ) {
                    err(1, "Cannot write elemtn to out file");
                }
            }
        }
    }

    if (read_bytes == -1) {
        err(1, "Error reading from commands file");
    }

    close(commands);
    close(data);
    close(out);


    return 0;
}
