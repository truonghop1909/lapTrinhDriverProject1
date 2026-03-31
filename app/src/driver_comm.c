#include "../include/common.h"
#include "../include/driver_comm.h"
#include "../include/driver_protocol.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DRIVER_PATH "/dev/student_driver"

static int open_driver(void) {
    int fd = open(DRIVER_PATH, O_RDWR);
    if (fd < 0) {
        perror("Khong mo duoc /dev/student_driver");
    }
    return fd;
}

int driver_hash_sha1(const char *input, char *output_hex, size_t output_size) {
    int fd = open_driver();
    struct driver_text_packet pkt;

    if (fd < 0) return -1;
    if (!input || !output_hex || output_size == 0) {
        close(fd);
        return -1;
    }

    memset(&pkt, 0, sizeof(pkt));
    strncpy(pkt.input, input, sizeof(pkt.input) - 1);

    if (ioctl(fd, IOCTL_SHA1_HASH, &pkt) < 0) {
        perror("ioctl SHA1");
        close(fd);
        return -1;
    }

    strncpy(output_hex, pkt.output, output_size - 1);
    output_hex[output_size - 1] = '\0';

    close(fd);
    return 0;
}

int driver_normalize_name(const char *input, char *output, size_t output_size) {
    int fd = open_driver();
    struct driver_text_packet pkt;

    if (fd < 0) return -1;
    if (!input || !output || output_size == 0) {
        close(fd);
        return -1;
    }

    memset(&pkt, 0, sizeof(pkt));
    strncpy(pkt.input, input, sizeof(pkt.input) - 1);

    if (ioctl(fd, IOCTL_NORMALIZE_NAME, &pkt) < 0) {
        perror("ioctl NORMALIZE_NAME");
        close(fd);
        return -1;
    }

    strncpy(output, pkt.output, output_size - 1);
    output[output_size - 1] = '\0';

    close(fd);
    return 0;
}

int driver_normalize_student_id(const char *input, char *output, size_t output_size) {
    int fd = open_driver();
    struct driver_text_packet pkt;

    if (fd < 0) return -1;
    if (!input || !output || output_size == 0) {
        close(fd);
        return -1;
    }

    memset(&pkt, 0, sizeof(pkt));
    strncpy(pkt.input, input, sizeof(pkt.input) - 1);

    if (ioctl(fd, IOCTL_NORMALIZE_STUDENTID, &pkt) < 0) {
        perror("ioctl NORMALIZE_STUDENTID");
        close(fd);
        return -1;
    }

    strncpy(output, pkt.output, output_size - 1);
    output[output_size - 1] = '\0';

    close(fd);
    return 0;
}

int driver_validate_student_input(const char *student_id, const char *name) {
    int fd = open_driver();
    struct driver_validate_packet pkt;

    if (fd < 0) return -1;

    memset(&pkt, 0, sizeof(pkt));
    strncpy(pkt.student_id, student_id, sizeof(pkt.student_id) - 1);
    strncpy(pkt.name, name, sizeof(pkt.name) - 1);

    if (ioctl(fd, IOCTL_VALIDATE_INPUT, &pkt) < 0) {
        perror("ioctl VALIDATE_INPUT");
        close(fd);
        return -1;
    }

    if (pkt.status != 0) {
        printf("Validate fail: %s\n", pkt.message);
    }

    close(fd);
    return pkt.status;
}