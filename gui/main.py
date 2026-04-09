import customtkinter as ctk
from tkinter import ttk, messagebox
import theme
import backend_service as api
from backend_service import BackendError


APP_TITLE = "Student Management System"


class App(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title(APP_TITLE)
        self.geometry("1380x820")
        self.minsize(1180, 720)
        self.configure(fg_color=theme.COLORS["bg"])

        self.current_user = None
        self.current_frame = None

        self.show_login()

    def switch_frame(self, frame_class, *args):
        if self.current_frame is not None:
            self.current_frame.destroy()
        self.current_frame = frame_class(self, *args)
        self.current_frame.pack(fill="both", expand=True)

    def show_login(self):
        self.current_user = None
        self.switch_frame(LoginPage)

    def show_main(self, user):
        self.current_user = user
        self.switch_frame(MainPage, user)


class LoginPage(ctk.CTkFrame):
    def __init__(self, master):
        super().__init__(master, fg_color=theme.COLORS["bg"])

        wrapper = ctk.CTkFrame(
            self,
            width=1100,
            height=650,
            corner_radius=24,
            fg_color=theme.COLORS["panel"],
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        wrapper.place(relx=0.5, rely=0.5, anchor="center")

        left = ctk.CTkFrame(
            wrapper,
            width=460,
            height=650,
            corner_radius=0,
            fg_color=theme.COLORS["accent"],
        )
        left.place(x=0, y=0)

        right = ctk.CTkFrame(
            wrapper,
            width=640,
            height=650,
            corner_radius=0,
            fg_color=theme.COLORS["panel"],
        )
        right.place(x=460, y=0)

        ctk.CTkLabel(
            left,
            text="Student\nManagement\nSystem",
            font=("Arial", 36, "bold"),
            text_color="white",
            justify="left",
        ).place(x=42, y=90)

        ctk.CTkLabel(
            left,
            text="Hệ thống quản lý sinh viên\nkết hợp Linux Driver,\nđăng nhập, phân quyền,\nquản lý dữ liệu và log.",
            font=("Arial", 16),
            text_color="#dbeafe",
            justify="left",
        ).place(x=42, y=260)

        ctk.CTkLabel(
            right,
            text="Đăng nhập",
            font=("Arial", 32, "bold"),
            text_color=theme.COLORS["text"],
        ).place(x=70, y=90)

        ctk.CTkLabel(
            right,
            text="Nhập thông tin tài khoản để truy cập hệ thống",
            font=("Arial", 15),
            text_color=theme.COLORS["muted"],
        ).place(x=70, y=138)

        self.username_entry = ctk.CTkEntry(
            right,
            width=420,
            height=50,
            corner_radius=12,
            placeholder_text="Username",
            font=("Arial", 16),
        )
        self.username_entry.place(x=70, y=210)

        self.password_entry = ctk.CTkEntry(
            right,
            width=420,
            height=50,
            corner_radius=12,
            placeholder_text="Password",
            font=("Arial", 16),
            show="*",
        )
        self.password_entry.place(x=70, y=280)

        self.status_label = ctk.CTkLabel(
            right,
            text="",
            font=("Arial", 14),
            text_color=theme.COLORS["danger"],
        )
        self.status_label.place(x=70, y=345)

        login_btn = ctk.CTkButton(
            right,
            text="Đăng nhập",
            width=420,
            height=50,
            corner_radius=12,
            fg_color=theme.COLORS["accent"],
            hover_color=theme.COLORS["accent_hover"],
            font=("Arial", 16, "bold"),
            command=self.handle_login,
        )
        login_btn.place(x=70, y=390)

        ctk.CTkLabel(
            right,
            text="Tài khoản mặc định: admin / admin123",
            font=("Arial", 14),
            text_color=theme.COLORS["muted"],
        ).place(x=70, y=470)

        ctk.CTkLabel(
            right,
            text="GUI → Backend C → Driver Kernel",
            font=("Arial", 13),
            text_color=theme.COLORS["muted"],
        ).place(x=70, y=590)

        self.username_entry.bind("<Return>", lambda e: self.handle_login())
        self.password_entry.bind("<Return>", lambda e: self.handle_login())

    def handle_login(self):
        username = self.username_entry.get().strip()
        password = self.password_entry.get().strip()

        if not username or not password:
            self.status_label.configure(text="Vui lòng nhập đầy đủ username và password")
            return

        try:
            user = api.login(username, password)
            self.master.show_main(user)
        except BackendError as e:
            self.status_label.configure(text=str(e))


class MainPage(ctk.CTkFrame):
    def __init__(self, master, user):
        super().__init__(master, fg_color=theme.COLORS["bg"])
        self.user = user

        self.sidebar = Sidebar(self, user, self.change_page, self.logout)
        self.sidebar.pack(side="left", fill="y")

        self.right_area = ctk.CTkFrame(self, fg_color=theme.COLORS["bg"], corner_radius=0)
        self.right_area.pack(side="right", fill="both", expand=True)

        self.header = HeaderBar(self.right_area, user)
        self.header.pack(fill="x", padx=20, pady=(18, 8))

        self.page_container = ctk.CTkFrame(
            self.right_area,
            fg_color=theme.COLORS["bg"],
            corner_radius=0,
        )
        self.page_container.pack(fill="both", expand=True, padx=20, pady=(0, 10))

        self.status_bar = ctk.CTkLabel(
            self.right_area,
            text=f"Đăng nhập: {user['username']} | Role: {user['role']}",
            height=28,
            anchor="w",
            padx=12,
            fg_color=theme.COLORS["panel"],
            text_color=theme.COLORS["muted"],
            corner_radius=10,
        )
        self.status_bar.pack(fill="x", padx=20, pady=(0, 20))

        self.current_page = None
        self.change_page("dashboard")

    def logout(self):
        self.master.show_login()

    def clear_page(self):
        if self.current_page is not None:
            self.current_page.destroy()

    def change_page(self, page_name):
        self.sidebar.highlight(page_name)
        self.clear_page()

        if page_name == "dashboard":
            self.current_page = DashboardPage(self.page_container, self.user)
        elif page_name == "students":
            self.current_page = StudentsPage(self.page_container, self.user)
        elif page_name == "users":
            self.current_page = UsersPage(self.page_container, self.user)
        elif page_name == "password":
            self.current_page = ChangePasswordPage(self.page_container, self.user)
        elif page_name == "logs":
            self.current_page = LogsPage(self.page_container, self.user)
        else:
            self.current_page = DashboardPage(self.page_container, self.user)

        self.current_page.pack(fill="both", expand=True)


class HeaderBar(ctk.CTkFrame):
    def __init__(self, master, user):
        super().__init__(
            master,
            height=72,
            corner_radius=18,
            fg_color=theme.COLORS["panel"],
            border_width=1,
            border_color=theme.COLORS["border"],
        )

        self.grid_columnconfigure(0, weight=1)

        ctk.CTkLabel(
            self,
            text=APP_TITLE,
            font=("Arial", 24, "bold"),
            text_color=theme.COLORS["text"],
        ).grid(row=0, column=0, sticky="w", padx=22, pady=18)

        user_box = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel_2"],
            corner_radius=12,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        user_box.grid(row=0, column=1, sticky="e", padx=18)

        ctk.CTkLabel(
            user_box,
            text=f"{user['username']}  |  {user['role']}",
            font=("Arial", 14, "bold"),
            text_color=theme.COLORS["text"],
            padx=16,
            pady=10,
        ).pack()


class Sidebar(ctk.CTkFrame):
    def __init__(self, master, user, on_change_page, on_logout):
        super().__init__(
            master,
            width=250,
            fg_color=theme.COLORS["panel"],
            corner_radius=0,
            border_width=0,
        )
        self.on_change_page = on_change_page
        self.on_logout = on_logout
        self.buttons = {}

        ctk.CTkLabel(
            self,
            text="MENU",
            font=("Arial", 28, "bold"),
            text_color=theme.COLORS["text"],
        ).pack(pady=(28, 10), padx=24, anchor="w")

        ctk.CTkLabel(
            self,
            text="Điều hướng hệ thống",
            font=("Arial", 14),
            text_color=theme.COLORS["muted"],
        ).pack(pady=(0, 24), padx=24, anchor="w")

        self.add_nav_button("dashboard", "Dashboard")
        self.add_nav_button("students", "Quản lý sinh viên")
        self.add_nav_button("users", "Tạo user")
        self.add_nav_button("password", "Đổi mật khẩu")
        self.add_nav_button("logs", "Xem log")

        spacer = ctk.CTkFrame(self, fg_color="transparent")
        spacer.pack(expand=True, fill="both")

        ctk.CTkButton(
            self,
            text="Logout",
            height=46,
            corner_radius=12,
            fg_color=theme.COLORS["danger"],
            hover_color=theme.COLORS["danger_hover"],
            font=("Arial", 15, "bold"),
            command=self.on_logout,
        ).pack(fill="x", padx=20, pady=20)

        ctk.CTkLabel(
            self,
            text=f"{user['username']} • {user['role']}",
            font=("Arial", 13),
            text_color=theme.COLORS["muted"],
        ).pack(padx=20, pady=(0, 16), anchor="w")

    def add_nav_button(self, key, text):
        btn = ctk.CTkButton(
            self,
            text=text,
            height=46,
            corner_radius=12,
            fg_color=theme.COLORS["panel_2"],
            hover_color=theme.COLORS["accent_hover"],
            anchor="w",
            font=("Arial", 15, "bold"),
            command=lambda k=key: self.on_change_page(k),
        )
        btn.pack(fill="x", padx=20, pady=6)
        self.buttons[key] = btn

    def highlight(self, active_key):
        for key, btn in self.buttons.items():
            if key == active_key:
                btn.configure(fg_color=theme.COLORS["accent"])
            else:
                btn.configure(fg_color=theme.COLORS["panel_2"])


class PageTitle(ctk.CTkFrame):
    def __init__(self, master, title, subtitle=""):
        super().__init__(
            master,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
            height=92,
        )

        ctk.CTkLabel(
            self,
            text=title,
            font=("Arial", 28, "bold"),
            text_color=theme.COLORS["text"],
        ).pack(anchor="w", padx=24, pady=(18, 0))

        ctk.CTkLabel(
            self,
            text=subtitle,
            font=("Arial", 14),
            text_color=theme.COLORS["muted"],
        ).pack(anchor="w", padx=24, pady=(4, 16))


class StatCard(ctk.CTkFrame):
    def __init__(self, master, title, value, hint="", color=None):
        super().__init__(
            master,
            fg_color=theme.COLORS["card"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
            height=150,
        )

        accent = color or theme.COLORS["accent"]

        ctk.CTkLabel(
            self,
            text=title,
            font=("Arial", 15, "bold"),
            text_color=theme.COLORS["muted"],
        ).pack(anchor="w", padx=20, pady=(18, 8))

        ctk.CTkLabel(
            self,
            text=str(value),
            font=("Arial", 34, "bold"),
            text_color=theme.COLORS["text"],
        ).pack(anchor="w", padx=20)

        ctk.CTkLabel(
            self,
            text=hint,
            font=("Arial", 13),
            text_color=accent,
        ).pack(anchor="w", padx=20, pady=(8, 12))


class DashboardPage(ctk.CTkFrame):
    def __init__(self, master, user):
        super().__init__(master, fg_color=theme.COLORS["bg"])

        title = PageTitle(
            self,
            "Dashboard",
            "Tổng quan hệ thống quản lý sinh viên, tài khoản và nhật ký thao tác",
        )
        title.pack(fill="x", pady=(0, 18))

        stats = ctk.CTkFrame(self, fg_color="transparent")
        stats.pack(fill="x")

        for i in range(4):
            stats.grid_columnconfigure(i, weight=1)

        try:
            stat_data = api.dashboard(user["username"])
            students = api.list_students()
        except BackendError as e:
            stat_data = {"students": 0, "users": 0, "logs": 0, "avg_gpa": 0}
            students = []
            messagebox.showerror("Lỗi backend", str(e))

        StatCard(stats, "Tổng sinh viên", stat_data["students"], "Số bản ghi hiện có").grid(row=0, column=0, padx=8, sticky="nsew")
        StatCard(stats, "Tổng tài khoản", stat_data["users"], "Users trong hệ thống", color=theme.COLORS["success"]).grid(row=0, column=1, padx=8, sticky="nsew")
        StatCard(stats, "GPA trung bình", stat_data["avg_gpa"], "Trung bình toàn bộ sinh viên", color=theme.COLORS["warning"]).grid(row=0, column=2, padx=8, sticky="nsew")
        StatCard(stats, "Số log", stat_data["logs"], "Nhật ký thao tác hệ thống", color="#38bdf8").grid(row=0, column=3, padx=8, sticky="nsew")

        lower = ctk.CTkFrame(self, fg_color="transparent")
        lower.pack(fill="both", expand=True, pady=(18, 0))
        lower.grid_columnconfigure(0, weight=3)
        lower.grid_columnconfigure(1, weight=2)

        recent_frame = ctk.CTkFrame(
            lower,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        recent_frame.grid(row=0, column=0, sticky="nsew", padx=(0, 10))

        ctk.CTkLabel(
            recent_frame,
            text="Sinh viên gần đây",
            font=("Arial", 20, "bold"),
            text_color=theme.COLORS["text"],
        ).pack(anchor="w", padx=20, pady=(18, 4))

        ctk.CTkLabel(
            recent_frame,
            text="Dữ liệu xem nhanh để kiểm tra trạng thái hệ thống",
            font=("Arial", 13),
            text_color=theme.COLORS["muted"],
        ).pack(anchor="w", padx=20, pady=(0, 14))

        recent_students = students[-5:] if len(students) >= 5 else students
        if not recent_students:
            ctk.CTkLabel(
                recent_frame,
                text="Chưa có dữ liệu sinh viên",
                font=("Arial", 15),
                text_color=theme.COLORS["muted"],
            ).pack(anchor="w", padx=20, pady=20)
        else:
            for s in reversed(recent_students):
                row = ctk.CTkFrame(recent_frame, fg_color=theme.COLORS["panel_2"], corner_radius=12)
                row.pack(fill="x", padx=16, pady=6)
                ctk.CTkLabel(
                    row,
                    text=f"{s['id']}  •  {s['name']}",
                    font=("Arial", 15, "bold"),
                    text_color=theme.COLORS["text"],
                ).pack(anchor="w", padx=14, pady=(10, 2))
                ctk.CTkLabel(
                    row,
                    text=f"Năm sinh: {s['birth_year']}   |   Ngành: {s['major']}   |   GPA: {s['gpa']}",
                    font=("Arial", 13),
                    text_color=theme.COLORS["muted"],
                ).pack(anchor="w", padx=14, pady=(0, 10))

        info_frame = ctk.CTkFrame(
            lower,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        info_frame.grid(row=0, column=1, sticky="nsew", padx=(10, 0))

        ctk.CTkLabel(
            info_frame,
            text="Phiên làm việc hiện tại",
            font=("Arial", 20, "bold"),
            text_color=theme.COLORS["text"],
        ).pack(anchor="w", padx=20, pady=(18, 16))

        items = [
            ("Tài khoản", user["username"]),
            ("Vai trò", user["role"]),
            ("Môi trường", "GUI → Backend C → Driver"),
            ("Lưu trữ", "users.txt / students.csv / activity.log"),
            ("Xác thực", "Backend C gọi driver SHA1"),
        ]

        for label, value in items:
            box = ctk.CTkFrame(info_frame, fg_color=theme.COLORS["panel_2"], corner_radius=12)
            box.pack(fill="x", padx=16, pady=6)
            ctk.CTkLabel(
                box,
                text=label,
                font=("Arial", 13),
                text_color=theme.COLORS["muted"],
            ).pack(anchor="w", padx=14, pady=(10, 0))
            ctk.CTkLabel(
                box,
                text=value,
                font=("Arial", 16, "bold"),
                text_color=theme.COLORS["text"],
            ).pack(anchor="w", padx=14, pady=(2, 10))


class StudentsPage(ctk.CTkFrame):
    def __init__(self, master, user):
        super().__init__(master, fg_color=theme.COLORS["bg"])
        self.user = user

        title = PageTitle(
            self,
            "Quản lý sinh viên",
            "Tra cứu, thêm, sửa, xóa và sắp xếp dữ liệu sinh viên",
        )
        title.pack(fill="x", pady=(0, 18))

        top_bar = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        top_bar.pack(fill="x", pady=(0, 16))

        self.search_entry = ctk.CTkEntry(
            top_bar,
            width=280,
            height=42,
            corner_radius=12,
            placeholder_text="Tìm theo mã hoặc tên",
        )
        self.search_entry.pack(side="left", padx=18, pady=16)

        ctk.CTkButton(
            top_bar,
            text="Tìm",
            width=90,
            height=42,
            corner_radius=12,
            command=self.search_students,
        ).pack(side="left", padx=6)

        ctk.CTkButton(
            top_bar,
            text="Refresh",
            width=100,
            height=42,
            corner_radius=12,
            fg_color=theme.COLORS["panel_2"],
            command=self.load_table,
        ).pack(side="left", padx=6)

        ctk.CTkButton(
            top_bar,
            text="Sort GPA",
            width=100,
            height=42,
            corner_radius=12,
            fg_color=theme.COLORS["panel_2"],
            command=self.sort_gpa,
        ).pack(side="left", padx=6)

        ctk.CTkButton(
            top_bar,
            text="Sort Name",
            width=110,
            height=42,
            corner_radius=12,
            fg_color=theme.COLORS["panel_2"],
            command=self.sort_name,
        ).pack(side="left", padx=6)

        right_btns = ctk.CTkFrame(top_bar, fg_color="transparent")
        right_btns.pack(side="right", padx=18, pady=16)

        self.add_btn = ctk.CTkButton(
            right_btns,
            text="Thêm",
            width=90,
            height=42,
            corner_radius=12,
            command=self.add_student,
        )
        self.add_btn.pack(side="left", padx=4)

        self.edit_btn = ctk.CTkButton(
            right_btns,
            text="Sửa",
            width=90,
            height=42,
            corner_radius=12,
            fg_color=theme.COLORS["panel_2"],
            command=self.edit_student,
        )
        self.edit_btn.pack(side="left", padx=4)

        self.delete_btn = ctk.CTkButton(
            right_btns,
            text="Xóa",
            width=90,
            height=42,
            corner_radius=12,
            fg_color=theme.COLORS["danger"],
            hover_color=theme.COLORS["danger_hover"],
            command=self.delete_student,
        )
        self.delete_btn.pack(side="left", padx=4)

        if self.user["role"] != "ADMIN":
            self.add_btn.configure(state="disabled")
            self.edit_btn.configure(state="disabled")
            self.delete_btn.configure(state="disabled")

        table_wrapper = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        table_wrapper.pack(fill="both", expand=True)

        style = ttk.Style()
        style.theme_use("default")
        style.configure(
            "Treeview",
            background="#0f172a",
            foreground="#f8fafc",
            fieldbackground="#0f172a",
            rowheight=32,
            borderwidth=0,
            font=("Arial", 11),
        )
        style.configure(
            "Treeview.Heading",
            background="#1e293b",
            foreground="#f8fafc",
            font=("Arial", 11, "bold"),
            relief="flat",
        )
        style.map("Treeview", background=[("selected", "#1d4ed8")])

        columns = ("id", "name", "birth_year", "major", "gpa")
        self.tree = ttk.Treeview(table_wrapper, columns=columns, show="headings", height=18)

        configs = [
            ("id", "Mã SV", 110),
            ("name", "Họ tên", 260),
            ("birth_year", "Năm sinh", 100),
            ("major", "Ngành", 240),
            ("gpa", "GPA", 90),
        ]
        for key, text, width in configs:
            self.tree.heading(key, text=text)
            self.tree.column(key, width=width, anchor="center")

        self.tree.pack(fill="both", expand=True, padx=16, pady=16)

        self.load_table()

    def load_table(self, data=None):
        for item in self.tree.get_children():
            self.tree.delete(item)
        try:
            students = data if data is not None else api.list_students()
            for s in students:
                self.tree.insert("", "end", values=(s["id"], s["name"], s["birth_year"], s["major"], f"{float(s['gpa']):.2f}"))
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))

    def search_students(self):
        keyword = self.search_entry.get().strip()
        try:
            self.load_table(api.search_students(keyword))
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))

    def sort_gpa(self):
        try:
            api.sort_students_by_gpa(self.user["username"])
            self.load_table()
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))

    def sort_name(self):
        try:
            api.sort_students_by_name(self.user["username"])
            self.load_table()
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))

    def get_selected_student(self):
        selected = self.tree.selection()
        if not selected:
            return None
        values = self.tree.item(selected[0], "values")
        return {
            "id": values[0],
            "name": values[1],
            "birth_year": int(values[2]),
            "major": values[3],
            "gpa": float(values[4]),
        }

    def add_student(self):
        StudentDialog(self, "Thêm sinh viên", self.handle_add_submit)

    def handle_add_submit(self, data):
        try:
            api.add_student(
                self.user["username"],
                data["id"],
                data["name"],
                data["birth_year"],
                data["major"],
                data["gpa"],
            )
            messagebox.showinfo("Thành công", "Thêm sinh viên thành công")
            self.load_table()
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))

    def edit_student(self):
        student = self.get_selected_student()
        if not student:
            messagebox.showwarning("Cảnh báo", "Hãy chọn một sinh viên")
            return
        StudentDialog(self, "Sửa sinh viên", self.handle_edit_submit, student)

    def handle_edit_submit(self, data):
        try:
            api.update_student(
                self.user["username"],
                data["id"],
                data["name"],
                data["birth_year"],
                data["major"],
                data["gpa"],
            )
            messagebox.showinfo("Thành công", "Sửa sinh viên thành công")
            self.load_table()
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))

    def delete_student(self):
        student = self.get_selected_student()
        if not student:
            messagebox.showwarning("Cảnh báo", "Hãy chọn một sinh viên")
            return

        if not messagebox.askyesno("Xác nhận", f"Xóa sinh viên {student['id']}?"):
            return

        try:
            api.delete_student(self.user["username"], student["id"])
            messagebox.showinfo("Thành công", "Xóa sinh viên thành công")
            self.load_table()
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))

class StudentDialog(ctk.CTkToplevel):
    def __init__(self, master, title, on_submit, student=None):
        super().__init__(master)
        self.title(title)
        self.geometry("560x640")
        self.resizable(False, False)
        self.configure(fg_color=theme.COLORS["panel"])
        self.on_submit = on_submit

        self.transient(master.winfo_toplevel())
        self.lift()
        self.focus_force()
        self.after(100, lambda: self.attributes("-topmost", True))
        self.after(200, lambda: self.attributes("-topmost", False))
        self.grab_set()

        wrapper = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel"],
            corner_radius=0,
        )
        wrapper.pack(fill="both", expand=True, padx=28, pady=24)

        ctk.CTkLabel(
            wrapper,
            text=title,
            font=("Arial", 30, "bold"),
            text_color=theme.COLORS["text"],
        ).pack(anchor="w", pady=(0, 20))

        form = ctk.CTkFrame(wrapper, fg_color="transparent")
        form.pack(fill="both", expand=True)

        self.id_entry = self.make_entry(form, "Mã sinh viên")
        self.name_entry = self.make_entry(form, "Họ tên")
        self.birth_entry = self.make_entry(form, "Năm sinh")
        self.major_entry = self.make_entry(form, "Ngành")
        self.gpa_entry = self.make_entry(form, "GPA")

        if student:
            self.id_entry.insert(0, student["id"])
            self.id_entry.configure(state="disabled")
            self.name_entry.insert(0, student["name"])
            self.birth_entry.insert(0, str(student["birth_year"]))
            self.major_entry.insert(0, student["major"])
            self.gpa_entry.insert(0, str(student["gpa"]))

        button_bar = ctk.CTkFrame(wrapper, fg_color="transparent")
        button_bar.pack(fill="x", pady=(18, 0))

        ctk.CTkButton(
            button_bar,
            text="Đóng",
            width=130,
            height=48,
            corner_radius=12,
            fg_color=theme.COLORS["panel_2"],
            command=self.destroy,
        ).pack(side="left")

        ctk.CTkButton(
            button_bar,
            text="Lưu dữ liệu",
            width=150,
            height=48,
            corner_radius=12,
            fg_color=theme.COLORS["accent"],
            hover_color=theme.COLORS["accent_hover"],
            command=self.submit,
        ).pack(side="right")

        self.protocol("WM_DELETE_WINDOW", self.destroy)

    def make_entry(self, master, label):
        ctk.CTkLabel(
            master,
            text=label,
            font=("Arial", 14, "bold"),
            text_color=theme.COLORS["muted"],
        ).pack(anchor="w", pady=(8, 6))

        entry = ctk.CTkEntry(
            master,
            height=46,
            corner_radius=12,
            font=("Arial", 15)
        )
        entry.pack(fill="x", pady=(0, 10))
        return entry

    def submit(self):
        try:
            data = {
                "id": self.id_entry.get().strip(),
                "name": self.name_entry.get().strip(),
                "birth_year": int(self.birth_entry.get().strip()),
                "major": self.major_entry.get().strip(),
                "gpa": float(self.gpa_entry.get().strip()),
            }
        except ValueError:
            messagebox.showerror("Lỗi dữ liệu", "Năm sinh hoặc GPA không hợp lệ", parent=self)
            return

        if not data["id"] or not data["name"] or not data["major"]:
            messagebox.showerror("Lỗi dữ liệu", "Không được để trống mã, tên hoặc ngành", parent=self)
            return

        self.on_submit(data)
        self.destroy()

class UsersPage(ctk.CTkFrame):
    def __init__(self, master, user):
        super().__init__(master, fg_color=theme.COLORS["bg"])
        self.user = user

        title = PageTitle(
            self,
            "Tạo user mới",
            "Tạo thêm tài khoản hệ thống và phân quyền ADMIN hoặc USER",
        )
        title.pack(fill="x", pady=(0, 18))

        if self.user["role"] != "ADMIN":
            card = ctk.CTkFrame(
                self,
                fg_color=theme.COLORS["panel"],
                corner_radius=18,
                border_width=1,
                border_color=theme.COLORS["border"],
            )
            card.pack(fill="x")
            ctk.CTkLabel(
                card,
                text="Chỉ ADMIN mới được tạo user mới",
                font=("Arial", 22, "bold"),
                text_color=theme.COLORS["text"],
            ).pack(padx=24, pady=28)
            return

        card = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        card.pack(fill="x")

        inner = ctk.CTkFrame(card, fg_color="transparent")
        inner.pack(fill="x", padx=24, pady=24)

        self.username_entry = self.make_field(inner, "Username")
        self.password_entry = self.make_field(inner, "Password", password=True)

        ctk.CTkLabel(
            inner,
            text="Role",
            font=("Arial", 14, "bold"),
            text_color=theme.COLORS["muted"],
        ).pack(anchor="w", pady=(16, 6))

        self.role_combo = ctk.CTkComboBox(inner, values=["ADMIN", "USER"], height=46, corner_radius=12)
        self.role_combo.pack(fill="x")
        self.role_combo.set("USER")

        ctk.CTkButton(
            inner,
            text="Tạo user",
            height=48,
            corner_radius=12,
            fg_color=theme.COLORS["accent"],
            hover_color=theme.COLORS["accent_hover"],
            font=("Arial", 15, "bold"),
            command=self.create_user,
        ).pack(pady=(24, 0), anchor="e")

    def make_field(self, master, label, password=False):
        ctk.CTkLabel(
            master,
            text=label,
            font=("Arial", 14, "bold"),
            text_color=theme.COLORS["muted"],
        ).pack(anchor="w", pady=(10, 6))
        entry = ctk.CTkEntry(master, height=46, corner_radius=12, show="*" if password else "")
        entry.pack(fill="x")
        return entry

    def create_user(self):
        username = self.username_entry.get().strip()
        password = self.password_entry.get().strip()
        role = self.role_combo.get().strip()

        try:
            api.create_user(self.user["username"], username, password, role)
            messagebox.showinfo("Thành công", "Tạo user thành công")
            self.username_entry.delete(0, "end")
            self.password_entry.delete(0, "end")
            self.role_combo.set("USER")
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))


class ChangePasswordPage(ctk.CTkFrame):
    def __init__(self, master, user):
        super().__init__(master, fg_color=theme.COLORS["bg"])
        self.user = user

        title = PageTitle(
            self,
            "Đổi mật khẩu",
            "Cập nhật mật khẩu đăng nhập cho tài khoản hiện tại",
        )
        title.pack(fill="x", pady=(0, 18))

        card = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        card.pack(fill="x")

        inner = ctk.CTkFrame(card, fg_color="transparent")
        inner.pack(fill="x", padx=24, pady=24)

        self.old_entry = self.make_field(inner, "Mật khẩu cũ")
        self.new_entry = self.make_field(inner, "Mật khẩu mới")

        ctk.CTkButton(
            inner,
            text="Đổi mật khẩu",
            height=48,
            corner_radius=12,
            fg_color=theme.COLORS["accent"],
            hover_color=theme.COLORS["accent_hover"],
            font=("Arial", 15, "bold"),
            command=self.change_password,
        ).pack(pady=(24, 0), anchor="e")

    def make_field(self, master, label):
        ctk.CTkLabel(
            master,
            text=label,
            font=("Arial", 14, "bold"),
            text_color=theme.COLORS["muted"],
        ).pack(anchor="w", pady=(10, 6))
        entry = ctk.CTkEntry(master, height=46, corner_radius=12, show="*")
        entry.pack(fill="x")
        return entry

    def change_password(self):
        old_pw = self.old_entry.get().strip()
        new_pw = self.new_entry.get().strip()

        try:
            api.change_password(self.user["username"], old_pw, new_pw)
            messagebox.showinfo("Thành công", "Đổi mật khẩu thành công")
            self.old_entry.delete(0, "end")
            self.new_entry.delete(0, "end")
        except BackendError as e:
            messagebox.showerror("Lỗi", str(e))


class LogsPage(ctk.CTkFrame):
    def __init__(self, master, user):
        super().__init__(master, fg_color=theme.COLORS["bg"])
        self.user = user

        title = PageTitle(
            self,
            "Nhật ký hệ thống",
            "Theo dõi lịch sử thao tác đăng nhập, tạo user và quản lý sinh viên",
        )
        title.pack(fill="x", pady=(0, 18))

        if self.user["role"] != "ADMIN":
            card = ctk.CTkFrame(
                self,
                fg_color=theme.COLORS["panel"],
                corner_radius=18,
                border_width=1,
                border_color=theme.COLORS["border"],
            )
            card.pack(fill="x")
            ctk.CTkLabel(
                card,
                text="Chỉ ADMIN mới được xem log hệ thống",
                font=("Arial", 22, "bold"),
                text_color=theme.COLORS["text"],
            ).pack(padx=24, pady=28)
            return

        top_bar = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        top_bar.pack(fill="x", pady=(0, 12))

        ctk.CTkButton(
            top_bar,
            text="Reload log",
            width=120,
            height=42,
            corner_radius=12,
            command=self.load_logs,
        ).pack(side="right", padx=18, pady=16)

        box = ctk.CTkFrame(
            self,
            fg_color=theme.COLORS["panel"],
            corner_radius=18,
            border_width=1,
            border_color=theme.COLORS["border"],
        )
        box.pack(fill="both", expand=True)

        self.textbox = ctk.CTkTextbox(
            box,
            corner_radius=12,
            fg_color=theme.COLORS["panel_2"],
            text_color=theme.COLORS["text"],
            font=("Consolas", 13),
        )
        self.textbox.pack(fill="both", expand=True, padx=16, pady=16)

        self.load_logs()

    def load_logs(self):
        self.textbox.configure(state="normal")
        self.textbox.delete("1.0", "end")
        try:
            content = api.read_logs(self.user["username"])
            self.textbox.insert("1.0", content if content else "Chưa có log nào.")
        except BackendError as e:
            self.textbox.insert("1.0", str(e))
        self.textbox.configure(state="disabled")


if __name__ == "__main__":
    app = App()
    app.mainloop()