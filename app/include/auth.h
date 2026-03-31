#ifndef AUTH_H
#define AUTH_H

#include "common.h"

int load_users(User users[], int *count);
int save_users(User users[], int count);

int register_user(User users[], int *count);
int login_user(User users[], int count, Session *session);
int change_password(User users[], int count, Session *session);
int ensure_default_admin(User users[], int *count);

int is_admin(Session *session);

#endif