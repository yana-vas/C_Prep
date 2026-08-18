#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <err.h>

struct PatchEntry {
    uint16_t offset;
    uint8_t org_byte;
    uint8_t new_byte;
};

int main(int argc, char *argv[]) {
    if (argc != 4 ) {
        errx(1, "Invalid number or args.");
    }

    int f1 = open(argv[1], O_RDONLY);
    if (f1 == -1 ){
        err(1, "Cannot open f1");
    }

    int f2 = open(argv[2], O_RDONLY);
    if(f2 == -1) {
        err(1, "Cannot open f2");
    }

    int patch = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(patch == -1) {
        err(1, "Cannot open patch");
    }

    struct stat st_f1;
    if(fstat(f1, &st_f1) == -1) {
        err(1, "Cannot stat f1");
    }
    struct stat st_f2;
    if(fstat(f2, &st_f2) == -1) {
        err(1, "Cannot stat f2");
    }
    if(st_f1.st_size != st_f2.st_size) {
        errx(1, "Different size of f1 and f2");
    }
    if(st_f1.st_size > UINT16_MAX) {
        errx(1, "Too big size of f1");
    }

    uint16_t offset = 0;
    uint8_t b1, b2;
    ssize_t read_bytes;
    while((read_bytes = read(f1, &b1, sizeof(b1))) > 0) {
        if(read(f2, &b2, sizeof(b2)) == -1) {
            err(1, "Cannot read element from f2");
        }

        if(b1 != b2) {
            struct PatchEntry pt;
            pt.offset=offset;
            pt.org_byte=b1;
            pt.new_byte=b2;

            if(write(patch, &pt, sizeof(struct PatchEntry)) == -1){
                err(1, "Cannot write element to patch");
            }

        }
        offset++;
    }

    if(read_bytes == -1) {
        err(1, "Error reading from f1");
    }
    close(f1);
    close(f2);
    close(patch);

    return 0;
}
