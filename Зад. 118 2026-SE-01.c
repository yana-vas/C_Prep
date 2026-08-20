#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>

struct PacketHeader {
    uint32_t P;
    uint32_t N;
};

int main(int argc, char *argv[]) {

    if(argc != 3) {
        errx(1, "Invalid number of args");
    }

    int in = open(argv[1], O_RDONLY);
    int out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if(in == -1) {
        err(1, "Cannot open input file");
    }

    if(out == -1) {
        err(1, "Cannot open output file");
    }

    struct PacketHeader hdr;

    ssize_t read_bytes;
    while((read_bytes = read(in, &hdr, sizeof(hdr))) > 0) {

        if(lseek(out, hdr.P, SEEK_SET) == -1) {
            err(1, "Cannot lseek output file");
        }

        uint8_t buff[4096];
        uint32_t remaining = hdr.N;

        while(remaining > 0) {

            size_t to_read=sizeof(buff);
            if(remaining < sizeof(buff)) {
                to_read=remaining;
            }

            ssize_t r_b=read(in, buff, to_read);
            if(r_b == -1) {
                err(1, "Cannot read element from input file");
            }

            if(r_b == 0){
                errx(1, "Smth went wrong with the file while reading");
            }

            if(write(out, buff, r_b) == -1){
                err(1, "Cannot write element to output file");
            }

            remaining-=r_b;
        }

    }

    if(read_bytes == -1) {
        err(1, "Error reading from input file");
    }

    close(in);
    close(out);

    return 0;
}
