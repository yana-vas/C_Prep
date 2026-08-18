#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <err.h>

struct Entry {
    uint16_t offset;
    uint8_t length;
    uint8_t reserved;
};

int main(int argc, char *argv[]) {
    if ( argc != 5 ) {
        errx(1, "Invalid number of args.");
    }

    int f1_dat = open(argv[1], O_RDONLY);
    if ( f1_dat == -1 ) {
        err(1, "Cannot open f1 dat file");
    }

    int f1_idx = open(argv[2], O_RDONLY);
    if (f1_idx == -1) {
        err(1, "Cannot open f1 idx file");
    }

    int f2_dat = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (f2_dat == -1){
        err(1, "Cannot open f2 dat file");
    }

    int f2_idx = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (f2_idx == -1 ) {
        err(1, "Cannot open f2 idx file");
    }

    struct stat st_idx;

    if(fstat(f1_idx, &st_idx) == -1) {
        err(1, "Cannot stat f1 idx file");
    }
    if(st_idx.st_size % sizeof(struct Entry) != 0) {
        errx(1, "Invalid size of f1 idx file");
    }

    struct stat st_dat;
    if(fstat(f1_dat, &st_dat) == -1) {
        err(1, "Cannot stat f1 dat file");
    }
    if(st_dat.st_size % sizeof(uint8_t) != 0) {
        errx(1, "Invalid size of f1 dat file");
    }

    uint16_t current_f2_offset = 0;

    ssize_t read_bytes;
    struct Entry entry;

    while((read_bytes = read(f1_idx, &entry, sizeof(struct Entry))) > 0) {
        if ( (entry.offset + entry.length) > st_dat.st_size) {
            errx(1, "Something is wrong with the f1 idx file");
        }

        if (entry.length == 0) {
            continue;
        }

        if(lseek(f1_dat, entry.offset, SEEK_SET) == -1) {
            err(1, "Cannot lseek f1 dat file");
        }

        uint8_t first_byte;
        if(read(f1_dat, &first_byte, sizeof(first_byte)) == -1) {
            err(1, "Cannot read element from f1 dat file");
        }

        if (first_byte >= 'A' && first_byte <= 'Z') {
            if(write(f2_dat, &first_byte, sizeof(first_byte)) == -1) {
                err(1, "Cannot write element in f2 dat file");
            }

            if(entry.length > 1) {
                for(uint8_t i = 0; i < entry.length-1; i++) {
                    uint8_t next_byte;
                    if(read(f1_dat, &next_byte, sizeof(next_byte)) == -1) {
                        err(1, "Cannot read element from f1 dat file");
                    }
                    if(write(f2_dat, &next_byte, sizeof(next_byte)) == -1) {
                        err(1, "Cannot write element to f2 dat file");
                    }
                }
            }

            struct Entry new_entry;
            new_entry.offset = current_f2_offset;
            new_entry.length = entry.length;
            new_entry.reserved = 0;

            if(write(f2_idx, &new_entry, sizeof(new_entry)) == -1) {
                err(1, "Cannot write new entry to f2 idx");
            }
            current_f2_offset += entry.length;


        }
    }

    if (read_bytes == -1) {
        err(1, "Error reading from f1 idx");
    }

    close(f1_dat);
    close(f1_idx);
    close(f2_idx);
    close(f2_dat);

    return 0;
}
