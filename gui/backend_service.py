import json
import os
import subprocess

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKEND_BIN = os.path.join(BASE_DIR, "student_backend")


class BackendError(Exception):
    pass


def _run_backend(args):
    if not os.path.exists(BACKEND_BIN):
        raise BackendError("Chưa build student_backend")

    try:
        result = subprocess.run(
            [BACKEND_BIN] + args,
            capture_output=True,
            text=True,
            cwd=BASE_DIR,
            check=False,
        )
    except Exception as e:
        raise BackendError(f"Không gọi được backend: {e}")

    output = result.stdout.strip()
    if not output:
        err = result.stderr.strip() or "Backend không trả dữ liệu"
        raise BackendError(err)

    try:
        data = json.loads(output)
    except json.JSONDecodeError:
        raise BackendError(f"Backend trả dữ liệu không hợp lệ: {output}")

    if not data.get("ok"):
        raise BackendError(data.get("message", "Lỗi backend"))

    return data


def login(username: str, password: str):
    data = _run_backend(["login", username, password])
    return data["user"]


def dashboard(current_username: str):
    data = _run_backend(["dashboard", current_username])
    return data["stats"]


def create_user(current_username: str, username: str, password: str, role: str):
    _run_backend(["create-user", current_username, username, password, role])
    return True


def change_password(current_username: str, old_password: str, new_password: str):
    _run_backend(["change-password", current_username, old_password, new_password])
    return True


def list_students():
    data = _run_backend(["list-students"])
    return data["students"]


def search_students(keyword: str):
    data = _run_backend(["search-students", keyword])
    return data["students"]


def add_student(current_username: str, student_id: str, name: str, birth_year: int, major: str, gpa: float):
    _run_backend([
        "add-student",
        current_username,
        student_id,
        name,
        str(birth_year),
        major,
        str(gpa),
    ])
    return True


def update_student(current_username: str, student_id: str, name: str, birth_year: int, major: str, gpa: float):
    _run_backend([
        "update-student",
        current_username,
        student_id,
        name,
        str(birth_year),
        major,
        str(gpa),
    ])
    return True


def delete_student(current_username: str, student_id: str):
    _run_backend(["delete-student", current_username, student_id])
    return True


def sort_students_by_gpa(current_username: str):
    _run_backend(["sort-gpa", current_username])
    return True


def sort_students_by_name(current_username: str):
    _run_backend(["sort-name", current_username])
    return True


def read_logs(current_username: str):
    data = _run_backend(["view-logs", current_username])
    return "\n".join(data["logs"])