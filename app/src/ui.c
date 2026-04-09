#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ui.h"
#include "../include/auth.h"
#include "../include/student.h"
#include "../include/logger.h"
#include "../include/file_io.h"

static void ui_draw_title(const char *title) {
    clear();
    attron(A_BOLD);
    mvprintw(1, 4, "%s", title);
    attroff(A_BOLD);
    mvhline(2, 2, '=', COLS - 4);
}

static void ui_wait(void) {
    mvprintw(LINES - 2, 2, "Nhan phim bat ky de tiep tuc...");
    getch();
}

static void ui_message(const char *title, const char *message) {
    ui_draw_title(title);
    mvprintw(4, 4, "%s", message);
    ui_wait();
}

static void ui_get_string(int y, int x, const char *label, char *buffer, int size, int hidden) {
    int ch;
    int len = 0;

    mvprintw(y, x, "%s", label);
    move(y, x + (int)strlen(label));
    clrtoeol();
    refresh();

    memset(buffer, 0, size);

    if (!hidden) {
        echo();
        curs_set(1);
        getnstr(buffer, size - 1);
        noecho();
        curs_set(0);
        return;
    }

    noecho();
    curs_set(1);

    while ((ch = getch()) != '\n' && ch != KEY_ENTER) {
        if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && len > 0) {
            len--;
            buffer[len] = '\0';
            mvaddch(y, x + (int)strlen(label) + len, ' ');
            move(y, x + (int)strlen(label) + len);
        } else if (len < size - 1 && ch >= 32 && ch <= 126) {
            buffer[len++] = (char)ch;
            buffer[len] = '\0';
            mvaddch(y, x + (int)strlen(label) + len - 1, '*');
        }
        refresh();
    }

    curs_set(0);
}

static int ui_get_int(int y, int x, const char *label) {
    char buf[32];
    ui_get_string(y, x, label, buf, sizeof(buf), 0);
    return atoi(buf);
}

static float ui_get_float(int y, int x, const char *label) {
    char buf[32];
    ui_get_string(y, x, label, buf, sizeof(buf), 0);
    return (float)atof(buf);
}

static int ui_main_menu(Session *session) {
    char buf[16];

    ui_draw_title("HE THONG QUAN LY SINH VIEN");
    if (session->logged_in) {
        mvprintw(4, 4, "Dang nhap: %s (%s)", session->current_user.username, session->current_user.role);
    } else {
        mvprintw(4, 4, "Dang nhap: Chua dang nhap");
    }

    mvprintw(6, 4, "1. Dang nhap");
    mvprintw(7, 4, "2. Them user moi");
    mvprintw(8, 4, "3. Doi mat khau");
    mvprintw(9, 4, "4. Quan ly sinh vien");
    mvprintw(10, 4, "5. Xem log he thong");
    mvprintw(11, 4, "0. Thoat");

    ui_get_string(13, 4, "Chon: ", buf, sizeof(buf), 0);
    return atoi(buf);
}

static int ui_student_menu(Session *session) {
    char buf[16];

    ui_draw_title("MENU QUAN LY SINH VIEN");
    mvprintw(4, 4, "Tai khoan: %s (%s)", session->current_user.username, session->current_user.role);

    if (strcmp(session->current_user.role, ROLE_ADMIN) == 0) {
        mvprintw(6, 4, "1. Them sinh vien");
        mvprintw(7, 4, "2. Sua sinh vien");
        mvprintw(8, 4, "3. Xoa sinh vien");
        mvprintw(9, 4, "4. Hien thi danh sach");
        mvprintw(10, 4, "5. Tim theo ma");
        mvprintw(11, 4, "6. Tim theo ten");
        mvprintw(12, 4, "7. Sap xep theo GPA");
        mvprintw(13, 4, "8. Sap xep theo ten");
        mvprintw(14, 4, "9. Doc lai tu file");
        mvprintw(15, 4, "0. Quay lai");
    } else {
        mvprintw(6, 4, "1. Hien thi danh sach");
        mvprintw(7, 4, "2. Tim theo ma");
        mvprintw(8, 4, "3. Tim theo ten");
        mvprintw(9, 4, "4. Sap xep theo GPA");
        mvprintw(10, 4, "5. Sap xep theo ten");
        mvprintw(11, 4, "6. Doc lai tu file");
        mvprintw(12, 4, "0. Quay lai");
    }

    ui_get_string(17, 4, "Chon: ", buf, sizeof(buf), 0);
    return atoi(buf);
}

static void ui_show_students(const Student *list, int count, const char *title) {
    int per_page = LINES - 8;
    if (per_page < 1) per_page = 1;

    for (int start = 0; start < count || (count == 0 && start == 0); start += per_page) {
        ui_draw_title(title);

        mvprintw(4, 2,  "%-12s %-25s %-10s %-20s %-5s",
                 "Ma SV", "Ho ten", "Nam sinh", "Nganh", "GPA");
        mvhline(5, 2, '-', COLS - 4);

        if (count == 0) {
            mvprintw(7, 4, "Khong co du lieu.");
        } else {
            int row = 6;
            for (int i = start; i < count && i < start + per_page; i++) {
                mvprintw(row++, 2, "%-12s %-25s %-10d %-20s %-5.2f",
                         list[i].id,
                         list[i].name,
                         list[i].birth_year,
                         list[i].major,
                         list[i].gpa);
            }
        }

        if (count > per_page && start + per_page < count) {
            mvprintw(LINES - 2, 2, "Nhan phim bat ky de xem tiep...");
        } else {
            mvprintw(LINES - 2, 2, "Nhan phim bat ky de quay lai...");
        }
        getch();

        if (count == 0) break;
    }
}

static void ui_handle_login(User users[], int user_count, Session *session) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    ui_draw_title("DANG NHAP");
    ui_get_string(4, 4, "Username: ", username, sizeof(username), 0);
    ui_get_string(5, 4, "Password: ", password, sizeof(password), 1);

    if (login_with_credentials(users, user_count, session, username, password) == 0) {
        ui_message("THANH CONG", "Dang nhap thanh cong.");
    } else {
        ui_message("THAT BAI", "Sai username hoac password.");
    }
}

static void ui_handle_register(User users[], int *user_count, Session *session) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char role[MAX_ROLE];

    if (!session->logged_in || !is_admin(session)) {
        ui_message("TU CHOI", "Chi ADMIN moi duoc them user moi.");
        return;
    }

    ui_draw_title("THEM USER MOI");
    ui_get_string(4, 4, "Username: ", username, sizeof(username), 0);
    ui_get_string(5, 4, "Password: ", password, sizeof(password), 1);
    ui_get_string(6, 4, "Role (ADMIN/USER): ", role, sizeof(role), 0);

    if (register_user_with_data(users, user_count, username, password, role) == 0) {
        ui_message("THANH CONG", "Tao user moi thanh cong.");
    } else {
        ui_message("THAT BAI", "Khong tao duoc user. Kiem tra du lieu.");
    }
}

static void ui_handle_change_password(User users[], int user_count, Session *session) {
    char old_password[MAX_PASSWORD];
    char new_password[MAX_PASSWORD];

    if (!session->logged_in) {
        ui_message("TU CHOI", "Ban can dang nhap truoc.");
        return;
    }

    ui_draw_title("DOI MAT KHAU");
    ui_get_string(4, 4, "Mat khau cu: ", old_password, sizeof(old_password), 1);
    ui_get_string(5, 4, "Mat khau moi: ", new_password, sizeof(new_password), 1);

    if (change_password_with_data(users, user_count, session, old_password, new_password) == 0) {
        ui_message("THANH CONG", "Doi mat khau thanh cong.");
    } else {
        ui_message("THAT BAI", "Khong doi duoc mat khau.");
    }
}

static void ui_handle_add_student(Session *session) {
    char id[64], name[128], major[64];

    int birth_year;
    float gpa;

    ui_draw_title("THEM SINH VIEN");
    ui_get_string(4, 4, "Ma sinh vien: ", id, sizeof(id), 0);
    ui_get_string(5, 4, "Ho ten: ", name, sizeof(name), 0);
    birth_year = ui_get_int(6, 4, "Nam sinh: ");
    ui_get_string(7, 4, "Nganh: ", major, sizeof(major), 0);
    gpa = ui_get_float(8, 4, "GPA: ");

    if (student_add(session, id, name, birth_year, major, gpa) == 0) {
        ui_message("THANH CONG", "Them sinh vien thanh cong.");
    } else {
        ui_message("THAT BAI", "Khong them duoc sinh vien.");
    }
}

static void ui_handle_edit_student(Session *session) {
    char id[64], name[128], major[64];
    int birth_year;
    float gpa;

    ui_draw_title("SUA SINH VIEN");
    ui_get_string(4, 4, "Ma sinh vien can sua: ", id, sizeof(id), 0);
    ui_get_string(5, 4, "Ten moi: ", name, sizeof(name), 0);
    birth_year = ui_get_int(6, 4, "Nam sinh moi: ");
    ui_get_string(7, 4, "Nganh moi: ", major, sizeof(major), 0);
    gpa = ui_get_float(8, 4, "GPA moi: ");

    if (student_update_by_id(session, id, name, birth_year, major, gpa) == 0) {
        ui_message("THANH CONG", "Sua sinh vien thanh cong.");
    } else {
        ui_message("THAT BAI", "Khong sua duoc sinh vien.");
    }
}

static void ui_handle_delete_student(Session *session) {
    char id[64];

    ui_draw_title("XOA SINH VIEN");
    ui_get_string(4, 4, "Ma sinh vien can xoa: ", id, sizeof(id), 0);

    if (student_delete_by_id(session, id) == 0) {
        ui_message("THANH CONG", "Xoa sinh vien thanh cong.");
    } else {
        ui_message("THAT BAI", "Khong xoa duoc sinh vien.");
    }
}

static void ui_handle_show_all_students(void) {
    Student temp[MAX_STUDENTS];
    int count = student_get_count();

    for (int i = 0; i < count; i++) {
        const Student *s = student_get_at(i);
        if (s) temp[i] = *s;
    }

    ui_show_students(temp, count, "DANH SACH SINH VIEN");
}

static void ui_handle_find_by_id(void) {
    char id[64];
    int idx;
    Student temp[1];

    ui_draw_title("TIM SINH VIEN THEO MA");
    ui_get_string(4, 4, "Nhap ma sinh vien: ", id, sizeof(id), 0);

    idx = student_find_index_by_id(id);
    if (idx == -1) {
        ui_message("KET QUA", "Khong tim thay sinh vien.");
        return;
    }

    {
        const Student *s = student_get_at(idx);
        if (!s) {
            ui_message("KET QUA", "Khong tim thay sinh vien.");
            return;
        }
        temp[0] = *s;
    }

    ui_show_students(temp, 1, "KET QUA TIM KIEM");
}

static void ui_handle_find_by_name(void) {
    char keyword[128];
    Student results[MAX_STUDENTS];
    int found;

    ui_draw_title("TIM SINH VIEN THEO TEN");
    ui_get_string(4, 4, "Nhap tu khoa: ", keyword, sizeof(keyword), 0);

    found = student_search_by_name(keyword, results, MAX_STUDENTS);
    ui_show_students(results, found, "KET QUA TIM KIEM");
}

static void ui_handle_view_logs(Session *session) {
    FILE *fp;
    char lines[1000][256];
    int count = 0;
    int per_page = LINES - 6;

    if (!session->logged_in || !is_admin(session)) {
        ui_message("TU CHOI", "Chi ADMIN moi duoc xem log.");
        return;
    }

    fp = fopen(LOG_FILE, "r");
    if (!fp) {
        ui_message("LOI", "Khong mo duoc file log.");
        return;
    }

    while (count < 1000 && fgets(lines[count], sizeof(lines[count]), fp) != NULL) {
        count++;
    }
    fclose(fp);

    if (per_page < 1) per_page = 1;

    for (int start = 0; start < count || (count == 0 && start == 0); start += per_page) {
        ui_draw_title("NHAT KY HE THONG");

        if (count == 0) {
            mvprintw(4, 4, "Chua co log nao.");
        } else {
            int row = 4;
            for (int i = start; i < count && i < start + per_page; i++) {
                mvprintw(row++, 2, "%s", lines[i]);
            }
        }

        if (count > per_page && start + per_page < count) {
            mvprintw(LINES - 2, 2, "Nhan phim bat ky de xem tiep...");
        } else {
            mvprintw(LINES - 2, 2, "Nhan phim bat ky de quay lai...");
        }
        getch();

        if (count == 0) break;
    }
}

static void ui_run_student_menu(Session *session) {
    int choice;

    while (1) {
        student_load_all();
        choice = ui_student_menu(session);

        if (strcmp(session->current_user.role, ROLE_ADMIN) == 0) {
            switch (choice) {
                case 1: ui_handle_add_student(session); break;
                case 2: ui_handle_edit_student(session); break;
                case 3: ui_handle_delete_student(session); break;
                case 4: ui_handle_show_all_students(); break;
                case 5: ui_handle_find_by_id(); break;
                case 6: ui_handle_find_by_name(); break;
                case 7:
                    student_sort_by_gpa(session);
                    ui_message("THANH CONG", "Da sap xep theo GPA.");
                    break;
                case 8:
                    student_sort_by_name(session);
                    ui_message("THANH CONG", "Da sap xep theo ten.");
                    break;
                case 9:
                    student_load_all();
                    ui_message("THANH CONG", "Da doc lai tu file.");
                    break;
                case 0: return;
                default:
                    ui_message("LOI", "Lua chon khong hop le.");
            }
        } else {
            switch (choice) {
                case 1: ui_handle_show_all_students(); break;
                case 2: ui_handle_find_by_id(); break;
                case 3: ui_handle_find_by_name(); break;
                case 4:
                    student_sort_by_gpa(session);
                    ui_message("THANH CONG", "Da sap xep theo GPA.");
                    break;
                case 5:
                    student_sort_by_name(session);
                    ui_message("THANH CONG", "Da sap xep theo ten.");
                    break;
                case 6:
                    student_load_all();
                    ui_message("THANH CONG", "Da doc lai tu file.");
                    break;
                case 0: return;
                default:
                    ui_message("LOI", "Lua chon khong hop le.");
            }
        }
    }
}

void ui_init(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

void ui_end(void) {
    endwin();
}

void ui_run_app(User users[], int *user_count, Session *session) {
    int choice;

    student_load_all();

    while (1) {
        choice = ui_main_menu(session);

        switch (choice) {
            case 1:
                ui_handle_login(users, *user_count, session);
                break;
            case 2:
                ui_handle_register(users, user_count, session);
                break;
            case 3:
                ui_handle_change_password(users, *user_count, session);
                break;
            case 4:
                if (!session->logged_in) {
                    ui_message("TU CHOI", "Ban can dang nhap truoc.");
                } else {
                    ui_run_student_menu(session);
                }
                break;
            case 5:
                ui_handle_view_logs(session);
                break;
            case 0:
                return;
            default:
                ui_message("LOI", "Lua chon khong hop le.");
        }
    }
}