import os
import hashlib
from typing import List, Dict, Optional

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA_DIR = os.path.join(BASE_DIR, "data")
USERS_FILE = os.path.join(DATA_DIR, "users.txt")
STUDENTS_FILE = os.path.join(DATA_DIR, "students.csv")
LOG_FILE = os.path.join(DATA_DIR, "activity.log")


def ensure_files():
    os.makedirs(DATA_DIR, exist_ok=True)
    for path in [USERS_FILE, STUDENTS_FILE, LOG_FILE]:
        if not os.path.exists(path):
            with open(path, "w", encoding="utf-8"):
                pass


def sha1_text(text: str) -> str:
    return hashlib.sha1(text.encode("utf-8")).hexdigest()


def log_action(username: str, action: str, detail: str):
    from datetime import datetime
    ensure_files()
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(LOG_FILE, "a", encoding="utf-8") as f:
        f.write(f"[{now}] USER={username} ACTION={action} DETAIL={detail}\n")


def load_users() -> List[Dict]:
    ensure_files()
    users = []
    with open(USERS_FILE, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split("|")
            if len(parts) == 3:
                users.append({
                    "username": parts[0],
                    "password_sha1": parts[1],
                    "role": parts[2]
                })
    return users


def save_users(users: List[Dict]):
    ensure_files()
    with open(USERS_FILE, "w", encoding="utf-8") as f:
        for u in users:
            f.write(f"{u['username']}|{u['password_sha1']}|{u['role']}\n")


def ensure_default_admin():
    users = load_users()
    if not users:
        users.append({
            "username": "admin",
            "password_sha1": sha1_text("admin123"),
            "role": "ADMIN"
        })
        save_users(users)
        log_action("system", "CREATE_DEFAULT_ADMIN", "admin/admin123")


def login(username: str, password: str) -> Optional[Dict]:
    users = load_users()
    password_sha1 = sha1_text(password)
    for u in users:
        if u["username"] == username and u["password_sha1"] == password_sha1:
            log_action(username, "LOGIN_SUCCESS", "Dang nhap thanh cong")
            return u
    log_action(username or "UNKNOWN", "LOGIN_FAIL", "Sai username hoac password")
    return None


def create_user(current_user: Dict, username: str, password: str, role: str) -> bool:
    if not current_user or current_user.get("role") != "ADMIN":
        return False
    users = load_users()
    for u in users:
        if u["username"] == username:
            return False
    if role not in ["ADMIN", "USER"]:
        return False
    users.append({
        "username": username,
        "password_sha1": sha1_text(password),
        "role": role
    })
    save_users(users)
    log_action(current_user["username"], "REGISTER_USER", f"Tao user {username}")
    return True


def change_password(current_user: Dict, old_password: str, new_password: str) -> bool:
    users = load_users()
    old_sha1 = sha1_text(old_password)
    new_sha1 = sha1_text(new_password)

    for u in users:
        if u["username"] == current_user["username"]:
            if u["password_sha1"] != old_sha1:
                return False
            u["password_sha1"] = new_sha1
            save_users(users)
            log_action(current_user["username"], "CHANGE_PASSWORD", "Doi mat khau")
            return True
    return False


def normalize_name(name: str) -> str:
    words = " ".join(name.strip().split()).lower().split(" ")
    return " ".join(w.capitalize() for w in words if w)


def normalize_student_id(student_id: str) -> str:
    return "".join(student_id.strip().split()).upper()


def load_students() -> List[Dict]:
    ensure_files()
    students = []
    with open(STUDENTS_FILE, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split(",")
            if len(parts) == 5:
                students.append({
                    "id": parts[0],
                    "name": parts[1],
                    "birth_year": int(parts[2]),
                    "major": parts[3],
                    "gpa": float(parts[4]),
                })
    return students


def save_students(students: List[Dict]):
    ensure_files()
    with open(STUDENTS_FILE, "w", encoding="utf-8") as f:
        for s in students:
            f.write(f"{s['id']},{s['name']},{s['birth_year']},{s['major']},{s['gpa']:.2f}\n")


def add_student(current_user: Dict, student_id: str, name: str, birth_year: int, major: str, gpa: float) -> bool:
    if current_user.get("role") != "ADMIN":
        return False

    student_id = normalize_student_id(student_id)
    name = normalize_name(name)

    if not student_id or not name:
        return False
    if birth_year < 1980 or birth_year > 2026:
        return False
    if gpa < 0 or gpa > 4:
        return False

    students = load_students()
    for s in students:
        if s["id"] == student_id:
            return False

    students.append({
        "id": student_id,
        "name": name,
        "birth_year": birth_year,
        "major": major.strip(),
        "gpa": gpa,
    })
    save_students(students)
    log_action(current_user["username"], "ADD_STUDENT", student_id)
    return True


def update_student(current_user: Dict, student_id: str, name: str, birth_year: int, major: str, gpa: float) -> bool:
    if current_user.get("role") != "ADMIN":
        return False

    students = load_students()
    target_id = normalize_student_id(student_id)

    for s in students:
        if s["id"] == target_id:
            s["name"] = normalize_name(name)
            s["birth_year"] = birth_year
            s["major"] = major.strip()
            s["gpa"] = gpa
            save_students(students)
            log_action(current_user["username"], "EDIT_STUDENT", target_id)
            return True
    return False


def delete_student(current_user: Dict, student_id: str) -> bool:
    if current_user.get("role") != "ADMIN":
        return False

    target_id = normalize_student_id(student_id)
    students = load_students()
    new_students = [s for s in students if s["id"] != target_id]

    if len(new_students) == len(students):
        return False

    save_students(new_students)
    log_action(current_user["username"], "DELETE_STUDENT", target_id)
    return True


def search_students(keyword: str) -> List[Dict]:
    students = load_students()
    keyword = keyword.strip().lower()
    if not keyword:
        return students
    return [
        s for s in students
        if keyword in s["id"].lower() or keyword in s["name"].lower()
    ]


def sort_students_by_name():
    students = load_students()
    students.sort(key=lambda x: x["name"])
    save_students(students)


def sort_students_by_gpa():
    students = load_students()
    students.sort(key=lambda x: x["gpa"], reverse=True)
    save_students(students)


def read_logs() -> str:
    ensure_files()
    with open(LOG_FILE, "r", encoding="utf-8") as f:
        return f.read()