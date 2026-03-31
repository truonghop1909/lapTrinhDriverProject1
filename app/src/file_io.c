#include "../include/common.h"
#include "../include/file_io.h"

int ensure_data_files_exist(void) {
    FILE *fp;

    fp = fopen(USERS_FILE, "a");
    if (!fp) {
        perror("Khong tao duoc users.txt");
        return -1;
    }
    fclose(fp);

    fp = fopen(STUDENTS_FILE, "a");
    if (!fp) {
        perror("Khong tao duoc students.csv");
        return -1;
    }
    fclose(fp);

    fp = fopen(LOG_FILE, "a");
    if (!fp) {
        perror("Khong tao duoc activity.log");
        return -1;
    }
    fclose(fp);

    return 0;
}