#ifndef UI_H
#define UI_H

#include "common.h"

void ui_init(void);
void ui_end(void);
void ui_run_app(User users[], int *user_count, Session *session);

#endif