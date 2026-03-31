#ifndef DRIVER_COMM_H
#define DRIVER_COMM_H

#include <stddef.h>

int driver_hash_sha1(const char *input, char *output_hex, size_t output_size);
int driver_normalize_name(const char *input, char *output, size_t output_size);
int driver_normalize_student_id(const char *input, char *output, size_t output_size);
int driver_validate_student_input(const char *student_id, const char *name);

#endif