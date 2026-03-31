#include "../include/common.h"
#include "../include/auth.h"
#include "../include/file_io.h"
#include "../include/student.h"
#include "../include/logger.h"

static void print_main_menu(void) {
    printf("\n=============================\n");
    printf(" HE THONG QUAN LY SINH VIEN\n");
    printf("=============================\n");
    printf("1. Dang nhap\n");
    printf("2. Them user moi\n");
    printf("3. Doi mat khau\n");
    printf("4. Quan ly sinh vien\n");
    printf("5. Xem log he thong\n");
    printf("0. Thoat\n");
    printf("Chon: ");
}

int main(void) {
    User users[MAX_USERS];
    int user_count = 0;
    Session session;
    int choice;

    session.logged_in = 0;

    if (ensure_data_files_exist() != 0) {
        return 1;
    }

    load_users(users, &user_count);
    ensure_default_admin(users, &user_count);

    while (1) {
        print_main_menu();

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Lua chon khong hop le.\n");
            continue;
        }
        getchar();

        switch (choice) {
            case 1:
                login_user(users, user_count, &session);
                break;
            case 2:
                if (!session.logged_in || !is_admin(&session)) {
                    printf("Chi ADMIN moi duoc them user moi.\n");
                } else {
                    register_user(users, &user_count);
                }
                break;
            case 3:
                change_password(users, user_count, &session);
                break;
            case 4:
                if (!session.logged_in) {
                    printf("Ban can dang nhap truoc.\n");
                } else {
                    student_menu(&session);
                }
                break;
            case 5:
                if (!session.logged_in || !is_admin(&session)) {
                    printf("Chi ADMIN moi duoc xem log.\n");
                } else {
                    view_logs();
                }
                break;
            case 0:
                printf("Tam biet.\n");
                return 0;
            default:
                printf("Lua chon khong hop le.\n");
        }
    }

    return 0;
}