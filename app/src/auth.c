#include "../include/common.h"
#include "../include/auth.h"
#include "../include/logger.h"
#include "../include/driver_comm.h"

int load_users(User users[], int *count) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        *count = 0;
        return 0;
    }

    *count = 0;
    while (*count < MAX_USERS &&
           fscanf(fp, "%49[^|]|%40[^|]|%9[^\n]\n",
                  users[*count].username,
                  users[*count].password_sha1,
                  users[*count].role) == 3) {
        (*count)++;
    }

    fclose(fp);
    return 0;
}

int save_users(User users[], int count) {
    FILE *fp = fopen(USERS_FILE, "w");
    if (!fp) {
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
    if (*count >= MAX_USERS) return -1;

    User admin;
    char hash[41];

    strcpy(admin.username, "admin");
    strcpy(admin.role, ROLE_ADMIN);

    if (driver_hash_sha1("admin123", hash, sizeof(hash)) != 0) {
        return -1;
    }

    strcpy(admin.password_sha1, hash);
    users[*count] = admin;
    (*count)++;

    save_users(users, *count);
    log_action("system", "CREATE_DEFAULT_ADMIN", "admin/admin123");

    return 0;
}

int is_admin(Session *session) {
    if (!session->logged_in) return 0;
    return strcmp(session->current_user.role, ROLE_ADMIN) == 0;
}

int login_with_credentials(User users[], int count, Session *session,
                           const char *username, const char *password) {
    char password_sha1[41];

    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        log_action("UNKNOWN", "LOGIN_FAIL", "Thong tin dang nhap rong");
        return -1;
    }

    if (driver_hash_sha1(password, password_sha1, sizeof(password_sha1)) != 0) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password_sha1, password_sha1) == 0) {
            session->current_user = users[i];
            session->logged_in = 1;
            log_action(username, "LOGIN_SUCCESS", "Dang nhap thanh cong");
            return 0;
        }
    }

    log_action(username, "LOGIN_FAIL", "Sai username hoac password");
    return -1;
}

int register_user_with_data(User users[], int *count,
                            const char *username,
                            const char *password,
                            const char *role) {
    User new_user;
    char password_sha1[41];

    if (!username || !password || !role) return -1;
    if (strlen(username) == 0 || strlen(password) == 0 || strlen(role) == 0) return -1;
    if (*count >= MAX_USERS) return -1;
    if (username_exists(users, *count, username)) return -1;

    if (strcmp(role, ROLE_ADMIN) != 0 && strcmp(role, ROLE_USER) != 0) return -1;

    if (driver_hash_sha1(password, password_sha1, sizeof(password_sha1)) != 0) {
        return -1;
    }

    memset(&new_user, 0, sizeof(new_user));
    strncpy(new_user.username, username, sizeof(new_user.username) - 1);
    strncpy(new_user.role, role, sizeof(new_user.role) - 1);
    strncpy(new_user.password_sha1, password_sha1, sizeof(new_user.password_sha1) - 1);

    users[*count] = new_user;
    (*count)++;

    save_users(users, *count);
    log_action(new_user.username, "REGISTER_USER", "Tao user moi");
    return 0;
}

int change_password_with_data(User users[], int count, Session *session,
                              const char *old_password,
                              const char *new_password) {
    char old_sha1[41];
    char new_sha1[41];

    if (!session->logged_in) return -1;
    if (!old_password || !new_password) return -1;
    if (strlen(old_password) == 0 || strlen(new_password) == 0) return -1;

    if (driver_hash_sha1(old_password, old_sha1, sizeof(old_sha1)) != 0 ||
        driver_hash_sha1(new_password, new_sha1, sizeof(new_sha1)) != 0) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, session->current_user.username) == 0) {
            if (strcmp(users[i].password_sha1, old_sha1) != 0) {
                return -1;
            }

            strcpy(users[i].password_sha1, new_sha1);
            session->current_user = users[i];
            save_users(users, count);

            log_action(session->current_user.username,
                       "CHANGE_PASSWORD",
                       "Doi mat khau thanh cong");
            return 0;
        }
    }

    return -1;
}