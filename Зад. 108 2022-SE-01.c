#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>

struct DataHeader {
    uint32_t magic;
    uint32_t count;
};

struct ComparatorHeader {
    uint32_t magic1;
    uint16_t magic2;
    uint16_t reserved;
    uint64_t count;
};

struct ComparatorData {
    uint16_t type;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t reserved3;
    uint32_t offset1;
    uint32_t offset2;
};

int main (int argc, char *argv[]) {

    if(argc != 3) {
        errx(1, "Invalid number of args");
    }

    int f_data = open(argv[1], O_RDWR);
    if (f_data == -1) {
        err(1, "Cannot open file data");
    }

    int f_comp = open(argv[2], O_RDONLY);
    if(f_comp == -1) {
        err(1, "Cannot open file comparator");
    }

    struct stat st_d;
    if(fstat(f_data, &st_d) == -1) {
        err(1, "Cannot fstat file data");
    }
    if((st_d.st_size % sizeof(struct DataHeader)) % sizeof(uint64_t) != 0) {
        errx(1, "Incorrect size");
    }

    struct stat st_c;
    if(fstat(f_comp, &st_c) == -1) {
        err(1, "Cannot fstat");
    }
    if((st_c.st_size % sizeof(struct ComparatorHeader)) % sizeof(struct ComparatorData) !=0) {
        err(1, "Invalid size");
    }

    struct DataHeader dh;
    if(read(f_data, &dh, sizeof(struct DataHeader)) == -1) {
        err(1, "Cannot read from file data");
    }

    struct ComparatorHeader ch;
    if(read(f_comp, &ch, sizeof(struct ComparatorHeader)) == -1) {
        err(1, "Cannot read from comparator file");
    }

    if(dh.magic != 0x21796F4A) {
        errx(1, "Invalid magic val in file data");
    }
    if(ch.magic1 != 0xAFBC7A37 || ch.magic2 != 0x1C27) {
        errx(1, "Invalid magic val in file comparator");
    }

    for(uint64_t i = 0; i < ch.count; i++) {
        struct ComparatorData cd;
        if(read(f_comp, &cd, sizeof(struct ComparatorData)) == -1) {
            err(1, "Cannot read data from comparator file");
        }

        if(cd.reserved1 != 0 || cd.reserved2 != 0 || cd.reserved3 != 0) {
            errx(1, "Reserved val should be 0");
        }

        if(lseek(f_data, sizeof(struct DataHeader) + cd.offset1 * sizeof(uint64_t), SEEK_SET) == -1) {
            err(1, "Cannot lseek");
        }

        uint64_t offset1_el;
        if(read(f_data, &offset1_el, sizeof(uint64_t)) == -1) {
            err(1, "Cannot read from file data");
        }

        if(lseek(f_data, sizeof(struct DataHeader) + cd.offset2 * sizeof(uint64_t), SEEK_SET) == -1) {
            err(1, "Cannot lseek");
        }

        uint64_t offset2_el;
        if(read(f_data, &offset2_el, sizeof(uint64_t)) == -1){
            err(1, "Cannot read from file data");
        }

        if(cd.type == 0) {
            if(offset1_el > offset2_el) {
                if(lseek(f_data, sizeof(struct DataHeader) + cd.offset1 * sizeof(uint64_t), SEEK_SET) == -1) {
                    err(1, "Cannot lseek");
                }

                if(write(f_data, &offset2_el, sizeof(uint64_t)) == -1){
                    err(1, "Cannot write to file data");
                }

                if(lseek(f_data, sizeof(struct DataHeader) + cd.offset2 * sizeof(uint64_t), SEEK_SET) == -1) {
                    err(1, "Cannot lseek");
                }

                if(write(f_data, &offset1_el, sizeof(uint64_t)) == -1) {
                    err(1, "Cannot write to file data");
                }
            }
        } else if (cd.type == 1) {
             if(offset1_el < offset2_el) {
                if(lseek(f_data, sizeof(struct DataHeader) + cd.offset1 * sizeof(uint64_t), SEEK_SET) == -1) {
                    err(1, "Cannot lseek");
                }

                if(write(f_data, &offset2_el, sizeof(uint64_t)) == -1){
                    err(1, "Cannot write to file data");
                }

                if(lseek(f_data, sizeof(struct DataHeader) + cd.offset2 * sizeof(uint64_t), SEEK_SET) == -1) {
                    err(1, "Cannot lseek");
                }

                if(write(f_data, &offset1_el, sizeof(uint64_t)) == -1) {
                    err(1, "Cannot write to file data");
                }
            }
        }


    }


    return 0;
}
