#ifndef DRIVER_PROTOCOL_H
#define DRIVER_PROTOCOL_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
typedef uint32_t __u32;
#endif

#define DRIVER_MAGIC 'S'

#define DRIVER_MAX_INPUT   256
#define DRIVER_MAX_OUTPUT  256

struct driver_text_packet {
    char input[DRIVER_MAX_INPUT];
    char output[DRIVER_MAX_OUTPUT];
    int status;
};

struct driver_validate_packet {
    char student_id[32];
    char name[128];
    int status;
    char message[128];
};

#define IOCTL_NORMALIZE_NAME      _IOWR(DRIVER_MAGIC, 1, struct driver_text_packet)
#define IOCTL_NORMALIZE_STUDENTID _IOWR(DRIVER_MAGIC, 2, struct driver_text_packet)
#define IOCTL_SHA1_HASH           _IOWR(DRIVER_MAGIC, 3, struct driver_text_packet)
#define IOCTL_VALIDATE_INPUT      _IOWR(DRIVER_MAGIC, 4, struct driver_validate_packet)

#endif
