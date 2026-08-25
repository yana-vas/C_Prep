#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>

struct Header {
    uint16_t magic;
    uint16_t filetype;
    uint32_t count;
};

int main(int argc, char *argv[]) {

    if(argc != 4) {
        errx(1, "Invalid number of args");
    }

    int list = open(argv[1], O_RDONLY);
    if(list == -1){
        err(1, "Cannot open the first file");
    }

    int data = open(argv[2], O_RDONLY);
    if(data == -1) {
        err(1, "Cannot open the second file");
    }

    int out = open(argv[3], O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(out == -1) {
        err(1, "Cannot open the third file");
    }

    struct Header h_list;
    if(read(list, &h_list, sizeof(struct Header)) == -1) {
        err(1, "Error reading from the header");
    }

    struct Header h_data;
    if(read(data, &h_data, sizeof(struct Header)) == -1) {
        err(1, "Error reading from the header of the data file");
    }

    if (h_list.magic != 0x5A4D || h_list.filetype != 1) {
        errx(1, "Incorrect header info in list file");
    }

    if(h_data.magic != 0x5A4D || h_data.filetype != 2) {
        errx(1, "Incorrect header info in data file");
    }

    struct Header h_out;
    h_out.magic=0x5A4D;
    h_out.filetype=3;
    h_out.count=h_list.count;

    if (write(out, &h_out, sizeof(struct Header)) == -1){
        err(1, "Error while writing the header to output file");
    }

    for (int i = 0; i < h_list.count; i++) {
        uint16_t val;
        if(read(list, &val, sizeof(uint16_t)) == -1) {
            err(1, "Cannot read a uint16_t num from the list file");
        }

        if(lseek(data, sizeof(struct Header) + i * sizeof(uint32_t), SEEK_SET) == -1) {
            err(1, "Cannot lseek data");
        }
        uint32_t num;
        if(read(data, &num, sizeof(uint32_t)) == -1) {
            err(1, "Cannot read from data");
        }

        if(lseek(out, sizeof(struct Header) + val * sizeof(uint64_t), SEEK_SET) == -1) {
            err(1, "Cannot lseek from out");
        }

        uint64_t new_val = num;
        if(write(out, &new_val, sizeof(uint64_t)) == -1){
            err(1, "Cannot write to out");
        }
    }

    close(list);
    close(data);
    close(out);
    return 0;
}
