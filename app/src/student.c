#include "../include/common.h"
#include "../include/student.h"
#include "../include/logger.h"
#include "../include/driver_comm.h"

static Student students[MAX_STUDENTS];
static int student_count = 0;

int student_load_all(void) {
    FILE *fp = fopen(STUDENTS_FILE, "r");
    if (!fp) {
        student_count = 0;
        return 0;
    }

    student_count = 0;
    while (student_count < MAX_STUDENTS &&
           fscanf(fp, "%19[^,],%99[^,],%d,%49[^,],%f\n",
                  students[student_count].id,
                  students[student_count].name,
                  &students[student_count].birth_year,
                  students[student_count].major,
                  &students[student_count].gpa) == 5) {
        student_count++;
    }

    fclose(fp);
    return 0;
}

int student_save_all(void) {
    FILE *fp = fopen(STUDENTS_FILE, "w");
    if (!fp) return -1;

    for (int i = 0; i < student_count; i++) {
        fprintf(fp, "%s,%s,%d,%s,%.2f\n",
                students[i].id,
                students[i].name,
                students[i].birth_year,
                students[i].major,
                students[i].gpa);
    }

    fclose(fp);
    return 0;
}

int student_get_count(void) {
    return student_count;
}

const Student *student_get_at(int index) {
    if (index < 0 || index >= student_count) return NULL;
    return &students[index];
}

int student_find_index_by_id(const char *student_id) {
    char normalized_id[64];

    if (!student_id) return -1;
    if (driver_normalize_student_id(student_id, normalized_id, sizeof(normalized_id)) != 0) {
        return -1;
    }

    for (int i = 0; i < student_count; i++) {
        if (strcmp(students[i].id, normalized_id) == 0) return i;
    }
    return -1;
}

int student_add(Session *session,
                const char *student_id,
                const char *name,
                int birth_year,
                const char *major,
                float gpa) {
    Student s;

    if (!session || !student_id || !name || !major) return -1;
    if (student_count >= MAX_STUDENTS) return -1;
    if (birth_year < 1980 || birth_year > 2026) return -1;
    if (gpa < 0.0f || gpa > 4.0f) return -1;

    memset(&s, 0, sizeof(s));

    if (driver_normalize_student_id(student_id, s.id, sizeof(s.id)) != 0 ||
        driver_normalize_name(name, s.name, sizeof(s.name)) != 0) {
        return -1;
    }

    if (driver_validate_student_input(s.id, s.name) != 0) {
        return -1;
    }

    if (student_find_index_by_id(s.id) != -1) {
        return -1;
    }

    s.birth_year = birth_year;
    strncpy(s.major, major, sizeof(s.major) - 1);
    s.gpa = gpa;

    students[student_count++] = s;
    student_save_all();

    log_action(session->current_user.username, "ADD_STUDENT", s.id);
    return 0;
}

int student_update_by_id(Session *session,
                         const char *student_id,
                         const char *new_name,
                         int birth_year,
                         const char *major,
                         float gpa) {
    int idx;

    if (!session || !student_id || !new_name || !major) return -1;
    if (birth_year < 1980 || birth_year > 2026) return -1;
    if (gpa < 0.0f || gpa > 4.0f) return -1;

    idx = student_find_index_by_id(student_id);
    if (idx == -1) return -1;

    if (driver_normalize_name(new_name, students[idx].name, sizeof(students[idx].name)) != 0) {
        return -1;
    }

    if (driver_validate_student_input(students[idx].id, students[idx].name) != 0) {
        return -1;
    }

    students[idx].birth_year = birth_year;
    strncpy(students[idx].major, major, sizeof(students[idx].major) - 1);
    students[idx].major[sizeof(students[idx].major) - 1] = '\0';
    students[idx].gpa = gpa;

    student_save_all();
    log_action(session->current_user.username, "EDIT_STUDENT", students[idx].id);
    return 0;
}

int student_delete_by_id(Session *session, const char *student_id) {
    int idx;

    if (!session || !student_id) return -1;

    idx = student_find_index_by_id(student_id);
    if (idx == -1) return -1;

    log_action(session->current_user.username, "DELETE_STUDENT", students[idx].id);

    for (int i = idx; i < student_count - 1; i++) {
        students[i] = students[i + 1];
    }
    student_count--;

    student_save_all();
    return 0;
}

int student_search_by_name(const char *keyword, Student results[], int max_results) {
    int found = 0;

    if (!keyword || !results || max_results <= 0) return 0;

    for (int i = 0; i < student_count && found < max_results; i++) {
        if (strstr(students[i].name, keyword) != NULL) {
            results[found++] = students[i];
        }
    }
    return found;
}

static int cmp_name(const void *a, const void *b) {
    const Student *x = (const Student *)a;
    const Student *y = (const Student *)b;
    return strcmp(x->name, y->name);
}

static int cmp_gpa_desc(const void *a, const void *b) {
    const Student *x = (const Student *)a;
    const Student *y = (const Student *)b;
    if (x->gpa < y->gpa) return 1;
    if (x->gpa > y->gpa) return -1;
    return 0;
}

void student_sort_by_name(Session *session) {
    qsort(students, student_count, sizeof(Student), cmp_name);
    student_save_all();
    if (session) log_action(session->current_user.username, "SORT_STUDENT", "Theo ten");
}

void student_sort_by_gpa(Session *session) {
    qsort(students, student_count, sizeof(Student), cmp_gpa_desc);
    student_save_all();
    if (session) log_action(session->current_user.username, "SORT_STUDENT", "Theo GPA");
}