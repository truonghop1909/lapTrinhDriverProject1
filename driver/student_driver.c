#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include "driver_protocol.h"

#define DEVICE_NAME "student_driver"
#define CLASS_NAME  "student_class"

static dev_t dev_num;
static struct cdev student_cdev;
static struct class *student_class = NULL;
static struct device *student_device = NULL;

/* ================= SHA1 IMPLEMENTATION ================= */

typedef struct {
    unsigned int state[5];
    unsigned int count[2];
    unsigned char buffer[64];
} SHA1_CTX;

static void sha1_transform(unsigned int state[5], const unsigned char buffer[64]) {
    unsigned int a, b, c, d, e, t, i;
    unsigned int block[80];

    for (i = 0; i < 16; i++) {
        block[i] = ((unsigned int)buffer[i * 4] << 24) |
                   ((unsigned int)buffer[i * 4 + 1] << 16) |
                   ((unsigned int)buffer[i * 4 + 2] << 8) |
                   ((unsigned int)buffer[i * 4 + 3]);
    }

    for (i = 16; i < 80; i++) {
        t = block[i - 3] ^ block[i - 8] ^ block[i - 14] ^ block[i - 16];
        block[i] = (t << 1) | (t >> 31);
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    for (i = 0; i < 80; i++) {
        unsigned int f, k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        t = ((a << 5) | (a >> 27)) + f + e + k + block[i];
        e = d;
        d = c;
        c = (b << 30) | (b >> 2);
        b = a;
        a = t;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

static void sha1_init(SHA1_CTX *ctx) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
}

static void sha1_update(SHA1_CTX *ctx, const unsigned char *data, unsigned int len) {
    unsigned int i, j;

    j = (ctx->count[0] >> 3) & 63;
    if ((ctx->count[0] += len << 3) < (len << 3))
        ctx->count[1]++;
    ctx->count[1] += (len >> 29);

    if ((j + len) > 63) {
        memcpy(&ctx->buffer[j], data, (i = 64 - j));
        sha1_transform(ctx->state, ctx->buffer);
        for (; i + 63 < len; i += 64) {
            sha1_transform(ctx->state, &data[i]);
        }
        j = 0;
    } else {
        i = 0;
    }

    memcpy(&ctx->buffer[j], &data[i], len - i);
}

static void sha1_final(unsigned char digest[20], SHA1_CTX *ctx) {
    unsigned int i;
    unsigned char finalcount[8];
    unsigned char c;

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((ctx->count[(i >= 4 ? 0 : 1)]
                        >> ((3 - (i & 3)) * 8)) & 255);
    }

    c = 0200;
    sha1_update(ctx, &c, 1);
    while ((ctx->count[0] & 504) != 448) {
        c = 0000;
        sha1_update(ctx, &c, 1);
    }

    sha1_update(ctx, finalcount, 8);

    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
            ((ctx->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
}

static void sha1_to_hex(const unsigned char digest[20], char out[41]) {
    static const char hex[] = "0123456789abcdef";
    int i;
    for (i = 0; i < 20; i++) {
        out[i * 2] = hex[(digest[i] >> 4) & 0xF];
        out[i * 2 + 1] = hex[digest[i] & 0xF];
    }
    out[40] = '\0';
}

static void sha1_string(const char *input, char output[41]) {
    SHA1_CTX ctx;
    unsigned char digest[20];

    sha1_init(&ctx);
    sha1_update(&ctx, (const unsigned char *)input, strlen(input));
    sha1_final(digest, &ctx);
    sha1_to_hex(digest, output);
}

/* ================= STRING HELPERS ================= */

static void normalize_spaces(const char *src, char *dst, size_t dst_size) {
    int i = 0, j = 0;
    int in_space = 1;

    while (src[i] != '\0' && j < dst_size - 1) {
        if (isspace(src[i])) {
            if (!in_space) {
                dst[j++] = ' ';
                in_space = 1;
            }
        } else {
            dst[j++] = src[i];
            in_space = 0;
        }
        i++;
    }

    if (j > 0 && dst[j - 1] == ' ')
        j--;

    dst[j] = '\0';
}

static void normalize_name(const char *src, char *dst, size_t dst_size) {
    char temp[DRIVER_MAX_OUTPUT];
    int i;

    normalize_spaces(src, temp, sizeof(temp));

    for (i = 0; temp[i]; i++) {
        temp[i] = tolower(temp[i]);
    }

    if (temp[0]) {
        temp[0] = toupper(temp[0]);
    }

    for (i = 1; temp[i]; i++) {
        if (temp[i - 1] == ' ') {
            temp[i] = toupper(temp[i]);
        }
    }

    strscpy(dst, temp, dst_size);
}

static void normalize_student_id(const char *src, char *dst, size_t dst_size) {
    int i = 0, j = 0;

    while (src[i] != '\0' && j < dst_size - 1) {
        if (!isspace(src[i])) {
            dst[j++] = toupper(src[i]);
        }
        i++;
    }

    dst[j] = '\0';
}

static int validate_student_input(const char *student_id, const char *name, char *message, size_t msg_size) {
    int i;
    size_t id_len = strlen(student_id);
    size_t name_len = strlen(name);

    if (id_len == 0) {
        strscpy(message, "Ma sinh vien khong duoc rong", msg_size);
        return -1;
    }

    if (name_len == 0) {
        strscpy(message, "Ten sinh vien khong duoc rong", msg_size);
        return -1;
    }

    if (id_len < 3 || id_len > 15) {
        strscpy(message, "Ma sinh vien phai tu 3 den 15 ky tu", msg_size);
        return -1;
    }

    for (i = 0; student_id[i]; i++) {
        if (!(isalnum(student_id[i]) || student_id[i] == '_')) {
            strscpy(message, "Ma sinh vien chi duoc chua chu, so hoac dau _", msg_size);
            return -1;
        }
    }

    strscpy(message, "Hop le", msg_size);
    return 0;
}

/* ================= FILE OPS ================= */

static int student_open(struct inode *inode, struct file *file) {
    pr_info("student_driver: open\n");
    return 0;
}

static int student_release(struct inode *inode, struct file *file) {
    pr_info("student_driver: release\n");
    return 0;
}

static long student_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct driver_text_packet text_pkt;
    struct driver_validate_packet val_pkt;

    switch (cmd) {
        case IOCTL_NORMALIZE_NAME:
            if (copy_from_user(&text_pkt, (void __user *)arg, sizeof(text_pkt)))
                return -EFAULT;
            normalize_name(text_pkt.input, text_pkt.output, sizeof(text_pkt.output));
            text_pkt.status = 0;
            if (copy_to_user((void __user *)arg, &text_pkt, sizeof(text_pkt)))
                return -EFAULT;
            return 0;

        case IOCTL_NORMALIZE_STUDENTID:
            if (copy_from_user(&text_pkt, (void __user *)arg, sizeof(text_pkt)))
                return -EFAULT;
            normalize_student_id(text_pkt.input, text_pkt.output, sizeof(text_pkt.output));
            text_pkt.status = 0;
            if (copy_to_user((void __user *)arg, &text_pkt, sizeof(text_pkt)))
                return -EFAULT;
            return 0;

        case IOCTL_SHA1_HASH:
            if (copy_from_user(&text_pkt, (void __user *)arg, sizeof(text_pkt)))
                return -EFAULT;
            sha1_string(text_pkt.input, text_pkt.output);
            text_pkt.status = 0;
            if (copy_to_user((void __user *)arg, &text_pkt, sizeof(text_pkt)))
                return -EFAULT;
            return 0;

        case IOCTL_VALIDATE_INPUT:
            if (copy_from_user(&val_pkt, (void __user *)arg, sizeof(val_pkt)))
                return -EFAULT;
            val_pkt.status = validate_student_input(val_pkt.student_id,
                                                    val_pkt.name,
                                                    val_pkt.message,
                                                    sizeof(val_pkt.message));
            if (copy_to_user((void __user *)arg, &val_pkt, sizeof(val_pkt)))
                return -EFAULT;
            return 0;

        default:
            return -EINVAL;
    }
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = student_open,
    .release = student_release,
    .unlocked_ioctl = student_ioctl,
};

static int __init student_driver_init(void) {
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("student_driver: alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&student_cdev, &fops);
    student_cdev.owner = THIS_MODULE;

    ret = cdev_add(&student_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        pr_err("student_driver: cdev_add failed\n");
        return ret;
    }

    student_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(student_class)) {
        cdev_del(&student_cdev);
        unregister_chrdev_region(dev_num, 1);
        pr_err("student_driver: class_create failed\n");
        return PTR_ERR(student_class);
    }

    student_device = device_create(student_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(student_device)) {
        class_destroy(student_class);
        cdev_del(&student_cdev);
        unregister_chrdev_region(dev_num, 1);
        pr_err("student_driver: device_create failed\n");
        return PTR_ERR(student_device);
    }

    pr_info("student_driver loaded: /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit student_driver_exit(void) {
    device_destroy(student_class, dev_num);
    class_destroy(student_class);
    cdev_del(&student_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("student_driver unloaded\n");
}

module_init(student_driver_init);
module_exit(student_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenAI");
MODULE_DESCRIPTION("Student management driver with normalize, SHA1, validate");
