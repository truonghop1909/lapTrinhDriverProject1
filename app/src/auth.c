#include "../include/common.h"
#include "../include/auth.h"
#include "../include/logger.h"
#include "../include/driver_comm.h"

static void trim_newline(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int load_users(User users[], int *count) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        *count = 0;
        return 0;
    }

    *count = 0;
    while (fscanf(fp, "%49[^|]|%40[^|]|%9[^\n]\n",
                  users[*count].username,
                  users[*count].password_sha1,
                  users[*count].role) == 3) {
        (*count)++;
        if (*count >= MAX_USERS) break;
    }

    fclose(fp);
    return 0;
}

int save_users(User users[], int count) {
    FILE *fp = fopen(USERS_FILE, "w");
    if (!fp) {
        perror("Khong mo duoc users.txt");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s\n",
                users[i].username,
                users[i].password_sha1,
                users[i].role);
    }

    fclose(fp);
    return 0;
}

static int username_exists(User users[], int count, const char *username) {
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int ensure_default_admin(User users[], int *count) {
    if (*count > 0) return 0;

    User admin;
    char hash[41];

    strcpy(admin.username, "admin");
    strcpy(admin.role, ROLE_ADMIN);

    if (driver_hash_sha1("admin123", hash, sizeof(hash)) != 0) {
        printf("Khong tao duoc admin mac dinh vi driver chua san sang.\n");
        return -1;
    }

    strcpy(admin.password_sha1, hash);
    users[*count] = admin;
    (*count)++;

    save_users(users, *count);
    log_action("system", "CREATE_DEFAULT_ADMIN", "admin/admin123");

    printf("Da tao admin mac dinh: username=admin, password=admin123\n");
    return 0;
}

int register_user(User users[], int *count) {
    if (*count >= MAX_USERS) {
        printf("Danh sach user da day.\n");
        return -1;
    }

    User new_user;
    char password[MAX_PASSWORD];
    char password_sha1[41];

    printf("Nhap username: ");
    fgets(new_user.username, sizeof(new_user.username), stdin);
    trim_newline(new_user.username);

    if (strlen(new_user.username) == 0) {
        printf("Username khong duoc rong.\n");
        return -1;
    }

    if (username_exists(users, *count, new_user.username)) {
        printf("Username da ton tai.\n");
        return -1;
    }

    printf("Nhap password: ");
    fgets(password, sizeof(password), stdin);
    trim_newline(password);

    if (strlen(password) == 0) {
        printf("Password khong duoc rong.\n");
        return -1;
    }

    printf("Nhap role (ADMIN/USER): ");
    fgets(new_user.role, sizeof(new_user.role), stdin);
    trim_newline(new_user.role);

    if (strcmp(new_user.role, ROLE_ADMIN) != 0 &&
        strcmp(new_user.role, ROLE_USER) != 0) {
        printf("Role khong hop le.\n");
        return -1;
    }

    if (driver_hash_sha1(password, password_sha1, sizeof(password_sha1)) != 0) {
        printf("Loi bam password.\n");
        return -1;
    }

    strcpy(new_user.password_sha1, password_sha1);
    users[*count] = new_user;
    (*count)++;

    save_users(users, *count);
    log_action(new_user.username, "REGISTER_USER", "Tao user moi");

    printf("Tao user thanh cong.\n");
    return 0;
}

int login_user(User users[], int count, Session *session) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char password_sha1[41];

    printf("Nhap username: ");
    fgets(username, sizeof(username), stdin);
    trim_newline(username);

    printf("Nhap password: ");
    fgets(password, sizeof(password), stdin);
    trim_newline(password);

    if (driver_hash_sha1(password, password_sha1, sizeof(password_sha1)) != 0) {
        printf("Loi bam password.\n");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password_sha1, password_sha1) == 0) {

            session->current_user = users[i];
            session->logged_in = 1;

            log_action(username, "LOGIN_SUCCESS", "Dang nhap thanh cong");
            printf("Dang nhap thanh cong. Role: %s\n", users[i].role);
            return 0;
        }
    }

    log_action(username, "LOGIN_FAIL", "Sai username hoac password");
    printf("Dang nhap that bai.\n");
    return -1;
}

int change_password(User users[], int count, Session *session) {
    if (!session->logged_in) {
        printf("Ban chua dang nhap.\n");
        return -1;
    }

    char old_password[MAX_PASSWORD];
    char new_password[MAX_PASSWORD];
    char old_sha1[41];
    char new_sha1[41];

    printf("Nhap mat khau cu: ");
    fgets(old_password, sizeof(old_password), stdin);
    trim_newline(old_password);

    printf("Nhap mat khau moi: ");
    fgets(new_password, sizeof(new_password), stdin);
    trim_newline(new_password);

    if (driver_hash_sha1(old_password, old_sha1, sizeof(old_sha1)) != 0 ||
    driver_hash_sha1(new_password, new_sha1, sizeof(new_sha1)) != 0) {
        printf("Loi bam password.\n");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, session->current_user.username) == 0) {
            if (strcmp(users[i].password_sha1, old_sha1) != 0) {
                printf("Mat khau cu khong dung.\n");
                return -1;
            }

            strcpy(users[i].password_sha1, new_sha1);
            session->current_user = users[i];

            save_users(users, count);
            log_action(session->current_user.username,
                       "CHANGE_PASSWORD",
                       "Doi mat khau thanh cong");

            printf("Doi mat khau thanh cong.\n");
            return 0;
        }
    }

    return -1;
}

int is_admin(Session *session) {
    if (!session->logged_in) return 0;
    return strcmp(session->current_user.role, ROLE_ADMIN) == 0;
}