#include <stdint.h>
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct PatchEntry {
    uint16_t offset;
    uint8_t org_byte;
    uint8_t new_byte;
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        errx(1, "Invalid number of args");
    }

    int patch = open(argv[1], O_RDONLY);
    if (patch == -1) {
        err(1, "Cannot open patch file");
    }

    int f1 = open(argv[2], O_RDONLY);
    if(f1 == -1) {
        err(1, "Cannot open f1");
    }

    int f2 = open(argv[3], O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (f2 == -1) {
        err(1, "Cannot open f2");
    }

    struct stat st_p;
    if(fstat(patch, &st_p) == -1) {
        err(1, "Cannot stat patch file");
    }
    if (st_p.st_size % sizeof(struct PatchEntry) != 0) {
        errx(1, "Wrong size of the file patch");
    }

    struct stat st_f1;
    if(fstat(f1, &st_f1) == -1){
        err(1, "Cannot stat f1");
    }
    if(st_f1.st_size % sizeof(uint8_t) != 0) {
        errx(1, "Wrong size of the file f1");
    }

    ssize_t read_bytes;
    uint8_t buff[4096];
    while( (read_bytes = read(f1, buff, sizeof(buff))) > 0) {
        if(write(f2, buff, read_bytes) == -1) {
            err(1, "Cannot write to f2");
        }

    }

    if (read_bytes == -1) {
        err(1, "Error reading f1");
    }

    struct PatchEntry pt;
    while( (read_bytes = read(patch, &pt, sizeof(pt))) > 0) {
        if (pt.offset >= st_f1.st_size) {
            errx(1, "Offset out of bound");
        }

        if(lseek(f1, pt.offset, SEEK_SET) == -1){
            err(1, "Cannot lseek f1");
        }

        uint8_t b1;
        if (read(f1, &b1, sizeof(b1)) == -1) {
            err(1, "Cannot read f1");
        }
        if (b1 != pt.org_byte) {
            errx(1, "Original byte does not match");
        }
        if(lseek(f2, pt.offset, SEEK_SET) == -1) {
            err(1, "Cannot lseek f2");
        }

        if(write(f2, &pt.new_byte, sizeof(b1)) == -1) {
            err(1, "Cannot write to f2");
        }
    }

    if( read_bytes == -1) {
        err(1, "Error readig from patch");
    }



    close(patch);
    close(f1);
    close(f2);

    return 0;
}
