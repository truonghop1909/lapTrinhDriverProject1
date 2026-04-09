#ifndef STUDENT_H
#define STUDENT_H

#include "common.h"

int student_load_all(void);
int student_save_all(void);

int student_get_count(void);
const Student *student_get_at(int index);

int student_add(Session *session,
                const char *student_id,
                const char *name,
                int birth_year,
                const char *major,
                float gpa);

int student_update_by_id(Session *session,
                         const char *student_id,
                         const char *new_name,
                         int birth_year,
                         const char *major,
                         float gpa);

int student_delete_by_id(Session *session, const char *student_id);

int student_find_index_by_id(const char *student_id);
int student_search_by_name(const char *keyword, Student results[], int max_results);

void student_sort_by_name(Session *session);
void student_sort_by_gpa(Session *session);

#endif