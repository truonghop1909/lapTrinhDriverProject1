#include "../include/common.h"
#include "../include/student.h"
#include "../include/logger.h"
#include "../include/driver_comm.h"

static Student students[MAX_STUDENTS];
static int student_count = 0;

static void trim_newline(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

static int load_students(void) {
    FILE *fp = fopen(STUDENTS_FILE, "r");
    if (!fp) {
        student_count = 0;
        return 0;
    }

    student_count = 0;
    while (fscanf(fp, "%19[^,],%99[^,],%d,%49[^,],%f\n",
                  students[student_count].id,
                  students[student_count].name,
                  &students[student_count].birth_year,
                  students[student_count].major,
                  &students[student_count].gpa) == 5) {
        student_count++;
        if (student_count >= MAX_STUDENTS) break;
    }

    fclose(fp);
    return 0;
}

static int save_students(void) {
    FILE *fp = fopen(STUDENTS_FILE, "w");
    if (!fp) {
        perror("Khong mo duoc students.csv");
        return -1;
    }

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

static int find_index_by_id(const char *id) {
    for (int i = 0; i < student_count; i++) {
        if (strcmp(students[i].id, id) == 0) return i;
    }
    return -1;
}

static void print_student_header(void) {
    printf("\n%-15s %-30s %-10s %-20s %-6s\n",
           "Ma SV", "Ho ten", "Nam sinh", "Nganh", "GPA");
    printf("--------------------------------------------------------------------------\n");
}

static void print_student(const Student *s) {
    printf("%-15s %-30s %-10d %-20s %-6.2f\n",
           s->id, s->name, s->birth_year, s->major, s->gpa);
}

static void show_all_students(void) {
    load_students();
    if (student_count == 0) {
        printf("Chua co sinh vien nao.\n");
        return;
    }

    print_student_header();
    for (int i = 0; i < student_count; i++) {
        print_student(&students[i]);
    }
}

static void add_student(Session *session) {
    Student s;
    char raw_id[64], raw_name[128];

    if (student_count >= MAX_STUDENTS) {
        printf("Danh sach sinh vien da day.\n");
        return;
    }

    printf("Nhap ma sinh vien: ");
    fgets(raw_id, sizeof(raw_id), stdin);
    trim_newline(raw_id);

    printf("Nhap ho ten: ");
    fgets(raw_name, sizeof(raw_name), stdin);
    trim_newline(raw_name);

    if (driver_normalize_student_id(raw_id, s.id, sizeof(s.id)) != 0 ||
    driver_normalize_name(raw_name, s.name, sizeof(s.name)) != 0) {
        printf("Loi goi driver.\n");
        return;
    }

    if (driver_validate_student_input(s.id, s.name) != 0) {
        return;
    }

    if (find_index_by_id(s.id) != -1) {
        printf("Ma sinh vien da ton tai.\n");
        return;
    }

    printf("Nhap nam sinh: ");
    scanf("%d", &s.birth_year);
    getchar();

    printf("Nhap nganh hoc: ");
    fgets(s.major, sizeof(s.major), stdin);
    trim_newline(s.major);

    printf("Nhap GPA: ");
    scanf("%f", &s.gpa);
    getchar();

    if (s.birth_year < 1980 || s.birth_year > 2026) {
        printf("Nam sinh khong hop le.\n");
        return;
    }

    if (s.gpa < 0.0f || s.gpa > 4.0f) {
        printf("GPA phai tu 0.0 den 4.0\n");
        return;
    }

    students[student_count++] = s;
    save_students();

    log_action(session->current_user.username, "ADD_STUDENT", s.id);
    printf("Them sinh vien thanh cong.\n");
}

static void edit_student(Session *session) {
    char id[64], raw_name[128];
    int idx;

    printf("Nhap ma sinh vien can sua: ");
    fgets(id, sizeof(id), stdin);
    trim_newline(id);

    {
        char normalized_id[64];
        if (driver_normalize_student_id(id, normalized_id, sizeof(normalized_id)) != 0) {
            printf("Loi driver.\n");
            return;
        }
        strcpy(id, normalized_id);
    }

    idx = find_index_by_id(id);
    if (idx == -1) {
        printf("Khong tim thay sinh vien.\n");
        return;
    }

    printf("Nhap ten moi: ");
    fgets(raw_name, sizeof(raw_name), stdin);
    trim_newline(raw_name);

    if (driver_normalize_name(raw_name, students[idx].name, sizeof(students[idx].name)) != 0) {
        printf("Loi driver.\n");
        return;
    }

    if (driver_validate_student_input(students[idx].id, students[idx].name) != 0) {
        return;
    }

    printf("Nhap nam sinh moi: ");
    scanf("%d", &students[idx].birth_year);
    getchar();

    printf("Nhap nganh moi: ");
    fgets(students[idx].major, sizeof(students[idx].major), stdin);
    trim_newline(students[idx].major);

    printf("Nhap GPA moi: ");
    scanf("%f", &students[idx].gpa);
    getchar();

    if (students[idx].birth_year < 1980 || students[idx].birth_year > 2026) {
        printf("Nam sinh khong hop le.\n");
        return;
    }

    if (students[idx].gpa < 0.0f || students[idx].gpa > 4.0f) {
        printf("GPA phai tu 0.0 den 4.0\n");
        return;
    }

    save_students();
    log_action(session->current_user.username, "EDIT_STUDENT", students[idx].id);
    printf("Sua sinh vien thanh cong.\n");
}

static void delete_student(Session *session) {
    char id[64];
    int idx;

    printf("Nhap ma sinh vien can xoa: ");
    fgets(id, sizeof(id), stdin);
    trim_newline(id);

    {
        char normalized_id[64];
        if (driver_normalize_student_id(id, normalized_id, sizeof(normalized_id)) != 0) {
            printf("Loi driver.\n");
            return;
        }
        strcpy(id, normalized_id);
    }

    idx = find_index_by_id(id);
    if (idx == -1) {
        printf("Khong tim thay sinh vien.\n");
        return;
    }

    for (int i = idx; i < student_count - 1; i++) {
        students[i] = students[i + 1];
    }
    student_count--;

    save_students();
    log_action(session->current_user.username, "DELETE_STUDENT", id);
    printf("Xoa sinh vien thanh cong.\n");
}

static void find_by_id(void) {
    char id[64];
    int idx;

    printf("Nhap ma sinh vien can tim: ");
    fgets(id, sizeof(id), stdin);
    trim_newline(id);

    {
        char normalized_id[64];
        if (driver_normalize_student_id(id, normalized_id, sizeof(normalized_id)) != 0) {
            printf("Loi driver.\n");
            return;
        }
        strcpy(id, normalized_id);
    }

    idx = find_index_by_id(id);
    if (idx == -1) {
        printf("Khong tim thay sinh vien.\n");
        return;
    }

    print_student_header();
    print_student(&students[idx]);
}

static void find_by_name(void) {
    char name[128];
    int found = 0;

    printf("Nhap ten can tim: ");
    fgets(name, sizeof(name), stdin);
    trim_newline(name);

    if (strlen(name) == 0) {
        printf("Ten khong duoc rong.\n");
        return;
    }

    print_student_header();
    for (int i = 0; i < student_count; i++) {
        if (strstr(students[i].name, name) != NULL) {
            print_student(&students[i]);
            found = 1;
        }
    }

    if (!found) {
        printf("Khong tim thay sinh vien nao.\n");
    }
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

static void sort_by_name(void) {
    qsort(students, student_count, sizeof(Student), cmp_name);
    save_students();
    printf("Da sap xep theo ten.\n");
}

static void sort_by_gpa(void) {
    qsort(students, student_count, sizeof(Student), cmp_gpa_desc);
    save_students();
    printf("Da sap xep theo GPA giam dan.\n");
}

static void print_student_menu_admin(void) {
    printf("\n========== MENU SINH VIEN (ADMIN) ==========\n");
    printf("1. Them sinh vien\n");
    printf("2. Sua sinh vien\n");
    printf("3. Xoa sinh vien\n");
    printf("4. Hien thi danh sach\n");
    printf("5. Tim theo ma\n");
    printf("6. Tim theo ten\n");
    printf("7. Sap xep theo GPA\n");
    printf("8. Sap xep theo ten\n");
    printf("9. Doc lai tu file\n");
    printf("0. Quay lai\n");
    printf("Chon: ");
}

static void print_student_menu_user(void) {
    printf("\n========== MENU SINH VIEN (USER) ==========\n");
    printf("1. Hien thi danh sach\n");
    printf("2. Tim theo ma\n");
    printf("3. Tim theo ten\n");
    printf("4. Sap xep theo GPA\n");
    printf("5. Sap xep theo ten\n");
    printf("6. Doc lai tu file\n");
    printf("0. Quay lai\n");
    printf("Chon: ");
}

void student_menu(Session *session) {
    int choice;
    load_students();

    if (strcmp(session->current_user.role, ROLE_ADMIN) == 0) {
        while (1) {
            print_student_menu_admin();
            if (scanf("%d", &choice) != 1) {
                while (getchar() != '\n');
                printf("Lua chon khong hop le.\n");
                continue;
            }
            getchar();

            switch (choice) {
                case 1: add_student(session); break;
                case 2: edit_student(session); break;
                case 3: delete_student(session); break;
                case 4: show_all_students(); break;
                case 5: find_by_id(); break;
                case 6: find_by_name(); break;
                case 7: sort_by_gpa(); break;
                case 8: sort_by_name(); break;
                case 9: load_students(); printf("Da doc lai tu file.\n"); break;
                case 0: return;
                default: printf("Lua chon khong hop le.\n");
            }
        }
    } else {
        while (1) {
            print_student_menu_user();
            if (scanf("%d", &choice) != 1) {
                while (getchar() != '\n');
                printf("Lua chon khong hop le.\n");
                continue;
            }
            getchar();

            switch (choice) {
                case 1: show_all_students(); break;
                case 2: find_by_id(); break;
                case 3: find_by_name(); break;
                case 4: sort_by_gpa(); break;
                case 5: sort_by_name(); break;
                case 6: load_students(); printf("Da doc lai tu file.\n"); break;
                case 0: return;
                default: printf("Lua chon khong hop le.\n");
            }
        }
    }
}