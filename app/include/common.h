#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define MAX_ROLE 10

#define MAX_ID 20
#define MAX_NAME 100
#define MAX_MAJOR 50

#define MAX_STUDENTS 1000
#define MAX_USERS 100

#define USERS_FILE "data/users.txt"
#define STUDENTS_FILE "data/students.csv"
#define LOG_FILE "data/activity.log"

#define ROLE_ADMIN "ADMIN"
#define ROLE_USER  "USER"

typedef struct {
    char username[MAX_USERNAME];
    char password_sha1[41];
    char role[MAX_ROLE];
} User;

typedef struct {
    char id[MAX_ID];
    char name[MAX_NAME];
    int birth_year;
    char major[MAX_MAJOR];
    float gpa;
} Student;

typedef struct {
    User current_user;
    int logged_in;
} Session;

#endif
