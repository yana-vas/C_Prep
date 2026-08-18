#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <err.h>

struct Pair {
    uint32_t a;
    uint32_t b;
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        errx(1, "Invalid number of args.");
    }

    int f1 = open(argv[1], O_RDONLY);
    if (f1 == -1) {
        err(1, "Cannot open f1.");
    }

    int f2 = open(argv[2], O_RDONLY);
    if (f2 == -1) {
        err(1, "Cannot open f2.");
    }

    int f3 = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (f3 == -1) {
        err(1, "Cannot open f3.");
    }

    struct stat st;
    if (fstat(f1, &st) == -1) {
        err(1, "Cannot stat f1");
    }
    if (st.st_size % sizeof(struct Pair) != 0) {
        errx(1, "Inavalid file format of f1 (size must be a multiple of %lu)", sizeof(struct Pair));
    }

    struct stat st2;
    if (fstat(f2, &st2) == -1) {
        err(1, "Cannot stat f2");
    }

    if( st2.st_size % sizeof(uint32_t) != 0 ) {
        errx(1, "Invalid file format of f2 (size must be a multiple of %lu)", sizeof(uint32_t));
    }

    struct Pair pair;
    ssize_t read_bytes;

    while ((read_bytes = read(f1, &pair, sizeof(struct Pair))) > 0) {
        if(lseek(f2, pair.a * sizeof(uint32_t), SEEK_SET) == -1) {
            err(1, "Cannot lseek in f2");
        }

        for (uint32_t i = 0; i < pair.b; i++) {
            uint32_t num;
            if (read(f2, &num, sizeof(num)) == -1) {
                err(1, "Cannot read element from f2");
            }

            if(write(f3, &num, sizeof(num)) == -1) {
                err(1, "Cannot write element to f3");
            }

        }
    }

    if (read_bytes == -1) {
        err(1, "Error reading from f1");
    }

    close(f1);
    close(f2);
    close(f3);

    return 0;
}
