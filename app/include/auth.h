#ifndef AUTH_H
#define AUTH_H

#include "common.h"

int load_users(User users[], int *count);
int save_users(User users[], int count);

int ensure_default_admin(User users[], int *count);
int is_admin(Session *session);

int login_with_credentials(User users[], int count, Session *session,
                           const char *username, const char *password);

int register_user_with_data(User users[], int *count,
                            const char *username,
                            const char *password,
                            const char *role);

int change_password_with_data(User users[], int count, Session *session,
                              const char *old_password,
                              const char *new_password);

#endif