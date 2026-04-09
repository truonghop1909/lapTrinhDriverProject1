#include "../include/common.h"
#include "../include/auth.h"
#include "../include/file_io.h"
#include "../include/ui.h"

int main(void) {
    User users[MAX_USERS];
    int user_count = 0;
    Session session;

    session.logged_in = 0;

    if (ensure_data_files_exist() != 0) {
        return 1;
    }

    load_users(users, &user_count);
    ensure_default_admin(users, &user_count);

    ui_init();
    ui_run_app(users, &user_count, &session);
    ui_end();

    return 0;
}