#include "../include/common.h"
#include "../include/auth.h"
#include "../include/student.h"
#include "../include/file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void json_escape(const char *src, char *dst, size_t dst_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j + 2 < dst_size; i++) {
        char c = src[i];
        if (c == '"' || c == '\\') {
            if (j + 2 >= dst_size) break;
            dst[j++] = '\\';
            dst[j++] = c;
        } else if (c == '\n') {
            if (j + 2 >= dst_size) break;
            dst[j++] = '\\';
            dst[j++] = 'n';
        } else if (c == '\r') {
            if (j + 2 >= dst_size) break;
            dst[j++] = '\\';
            dst[j++] = 'r';
        } else if (c == '\t') {
            if (j + 2 >= dst_size) break;
            dst[j++] = '\\';
            dst[j++] = 't';
        } else {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
}

static void print_error(const char *message) {
    char esc[512];
    json_escape(message ? message : "Unknown error", esc, sizeof(esc));
    printf("{\"ok\":false,\"message\":\"%s\"}\n", esc);
}

static void print_ok_message(const char *message) {
    char esc[512];
    json_escape(message ? message : "OK", esc, sizeof(esc));
    printf("{\"ok\":true,\"message\":\"%s\"}\n", esc);
}

static int build_session_by_username(const char *username, Session *session) {
    User users[MAX_USERS];
    int count = 0;

    memset(session, 0, sizeof(Session));

    if (load_users(users, &count) != 0) return -1;

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            session->current_user = users[i];
            session->logged_in = 1;
            return 0;
        }
    }
    return -1;
}

static void print_student_json(const Student *s) {
    char id[128], name[256], major[256];
    json_escape(s->id, id, sizeof(id));
    json_escape(s->name, name, sizeof(name));
    json_escape(s->major, major, sizeof(major));

    printf("{\"id\":\"%s\",\"name\":\"%s\",\"birth_year\":%d,\"major\":\"%s\",\"gpa\":%.2f}",
           id, name, s->birth_year, major, s->gpa);
}

static int count_log_lines(void) {
    FILE *fp = fopen(LOG_FILE, "r");
    int count = 0;
    char line[512];

    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp)) count++;
    fclose(fp);
    return count;
}

static void command_login(const char *username, const char *password) {
    User users[MAX_USERS];
    int count = 0;
    Session session;

    memset(&session, 0, sizeof(session));
    load_users(users, &count);
    ensure_default_admin(users, &count);

    if (login_with_credentials(users, count, &session, username, password) == 0) {
        char user_esc[128], role_esc[64];
        json_escape(session.current_user.username, user_esc, sizeof(user_esc));
        json_escape(session.current_user.role, role_esc, sizeof(role_esc));

        printf("{\"ok\":true,\"user\":{\"username\":\"%s\",\"role\":\"%s\"}}\n", user_esc, role_esc);
    } else {
        print_error("Sai username hoac password");
    }
}

static void command_dashboard(const char *current_username) {
    Session session;
    User users[MAX_USERS];
    int user_count = 0;
    int student_count;
    int log_count;
    double avg_gpa = 0.0;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung khong ton tai");
        return;
    }

    load_users(users, &user_count);
    student_load_all();
    student_count = student_get_count();
    log_count = count_log_lines();

    if (student_count > 0) {
        double total = 0.0;
        for (int i = 0; i < student_count; i++) {
            const Student *s = student_get_at(i);
            if (s) total += s->gpa;
        }
        avg_gpa = total / student_count;
    }

    printf("{\"ok\":true,\"stats\":{\"students\":%d,\"users\":%d,\"logs\":%d,\"avg_gpa\":%.2f}}\n",
           student_count, user_count, log_count, avg_gpa);
}

static void command_create_user(const char *current_username,
                                const char *new_username,
                                const char *new_password,
                                const char *role) {
    Session session;
    User users[MAX_USERS];
    int count = 0;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    if (!is_admin(&session)) {
        print_error("Chi ADMIN moi duoc tao user");
        return;
    }

    load_users(users, &count);

    if (register_user_with_data(users, &count, new_username, new_password, role) == 0) {
        print_ok_message("Tao user thanh cong");
    } else {
        print_error("Khong tao duoc user");
    }
}

static void command_change_password(const char *current_username,
                                    const char *old_password,
                                    const char *new_password) {
    Session session;
    User users[MAX_USERS];
    int count = 0;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    load_users(users, &count);

    if (change_password_with_data(users, count, &session, old_password, new_password) == 0) {
        print_ok_message("Doi mat khau thanh cong");
    } else {
        print_error("Khong doi duoc mat khau");
    }
}

static void command_list_students(void) {
    student_load_all();
    int count = student_get_count();

    printf("{\"ok\":true,\"students\":[");
    for (int i = 0; i < count; i++) {
        const Student *s = student_get_at(i);
        if (!s) continue;
        if (i > 0) printf(",");
        print_student_json(s);
    }
    printf("]}\n");
}

static void command_search_students(const char *keyword) {
    Student results[MAX_STUDENTS];
    int found;

    student_load_all();
    found = student_search_by_name(keyword, results, MAX_STUDENTS);

    if (found == 0) {
        int idx = student_find_index_by_id(keyword);
        if (idx >= 0) {
            const Student *s = student_get_at(idx);
            if (s) {
                results[0] = *s;
                found = 1;
            }
        }
    }

    printf("{\"ok\":true,\"students\":[");
    for (int i = 0; i < found; i++) {
        if (i > 0) printf(",");
        print_student_json(&results[i]);
    }
    printf("]}\n");
}

static void command_add_student(const char *current_username,
                                const char *student_id,
                                const char *name,
                                int birth_year,
                                const char *major,
                                float gpa) {
    Session session;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    student_load_all();

    if (student_add(&session, student_id, name, birth_year, major, gpa) == 0) {
        print_ok_message("Them sinh vien thanh cong");
    } else {
        print_error("Khong them duoc sinh vien");
    }
}

static void command_update_student(const char *current_username,
                                   const char *student_id,
                                   const char *name,
                                   int birth_year,
                                   const char *major,
                                   float gpa) {
    Session session;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    student_load_all();

    if (student_update_by_id(&session, student_id, name, birth_year, major, gpa) == 0) {
        print_ok_message("Sua sinh vien thanh cong");
    } else {
        print_error("Khong sua duoc sinh vien");
    }
}

static void command_delete_student(const char *current_username,
                                   const char *student_id) {
    Session session;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    student_load_all();

    if (student_delete_by_id(&session, student_id) == 0) {
        print_ok_message("Xoa sinh vien thanh cong");
    } else {
        print_error("Khong xoa duoc sinh vien");
    }
}

static void command_sort_gpa(const char *current_username) {
    Session session;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    student_load_all();
    student_sort_by_gpa(&session);
    print_ok_message("Da sap xep theo GPA");
}

static void command_sort_name(const char *current_username) {
    Session session;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    student_load_all();
    student_sort_by_name(&session);
    print_ok_message("Da sap xep theo ten");
}

static void command_view_logs(const char *current_username) {
    Session session;
    FILE *fp;
    char buf[512];
    char escaped[1024];
    int first = 1;

    if (build_session_by_username(current_username, &session) != 0) {
        print_error("Nguoi dung hien tai khong hop le");
        return;
    }

    if (!is_admin(&session)) {
        print_error("Chi ADMIN moi duoc xem log");
        return;
    }

    fp = fopen(LOG_FILE, "r");
    if (!fp) {
        print_error("Khong mo duoc file log");
        return;
    }

    printf("{\"ok\":true,\"logs\":[");
    while (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
        json_escape(buf, escaped, sizeof(escaped));
        if (!first) printf(",");
        printf("\"%s\"", escaped);
        first = 0;
    }
    printf("]}\n");

    fclose(fp);
}

static void print_usage(void) {
    print_error("Lenh khong hop le");
}

int main(int argc, char *argv[]) {
    if (ensure_data_files_exist() != 0) {
        print_error("Khong tao duoc file du lieu");
        return 1;
    }

    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "login") == 0) {
        if (argc != 4) return print_usage(), 1;
        command_login(argv[2], argv[3]);
        return 0;
    }

    if (strcmp(argv[1], "dashboard") == 0) {
        if (argc != 3) return print_usage(), 1;
        command_dashboard(argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "create-user") == 0) {
        if (argc != 6) return print_usage(), 1;
        command_create_user(argv[2], argv[3], argv[4], argv[5]);
        return 0;
    }

    if (strcmp(argv[1], "change-password") == 0) {
        if (argc != 5) return print_usage(), 1;
        command_change_password(argv[2], argv[3], argv[4]);
        return 0;
    }

    if (strcmp(argv[1], "list-students") == 0) {
        if (argc != 2) return print_usage(), 1;
        command_list_students();
        return 0;
    }

    if (strcmp(argv[1], "search-students") == 0) {
        if (argc != 3) return print_usage(), 1;
        command_search_students(argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "add-student") == 0) {
        if (argc != 8) return print_usage(), 1;
        command_add_student(argv[2], argv[3], argv[4], atoi(argv[5]), argv[6], (float)atof(argv[7]));
        return 0;
    }

    if (strcmp(argv[1], "update-student") == 0) {
        if (argc != 8) return print_usage(), 1;
        command_update_student(argv[2], argv[3], argv[4], atoi(argv[5]), argv[6], (float)atof(argv[7]));
        return 0;
    }

    if (strcmp(argv[1], "delete-student") == 0) {
        if (argc != 4) return print_usage(), 1;
        command_delete_student(argv[2], argv[3]);
        return 0;
    }

    if (strcmp(argv[1], "sort-gpa") == 0) {
        if (argc != 3) return print_usage(), 1;
        command_sort_gpa(argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "sort-name") == 0) {
        if (argc != 3) return print_usage(), 1;
        command_sort_name(argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "view-logs") == 0) {
        if (argc != 3) return print_usage(), 1;
        command_view_logs(argv[2]);
        return 0;
    }

    print_usage();
    return 1;
}