# He thong quan ly sinh vien su dung Linux Character Driver

## 1. Gioi thieu
De tai xay dung he thong quan ly danh sach sinh vien tren Linux, trong do mot so chuc nang cot loi duoc dua xuong kernel space thong qua character device driver.

He thong gom 2 phan:
- Ung dung user space viet bang C
- Linux character device driver viet bang kernel module

## 2. Chuc nang

### 2.1 Chuc nang tai khoan
- Dang nhap
- Doi mat khau
- Them user moi
- Password duoc luu duoi dang SHA1

### 2.2 Chuc nang sinh vien
- Them sinh vien
- Sua sinh vien
- Xoa sinh vien
- Hien thi danh sach
- Tim theo ma
- Tim theo ten
- Sap xep theo GPA
- Sap xep theo ten
- Ghi ra file
- Doc lai tu file

### 2.3 Chuc nang driver
- Chuan hoa ho ten
- Chuan hoa ma sinh vien
- Bam SHA1
- Kiem tra du lieu dau vao

### 2.4 Chuc nang log
- Log dang nhap
- Log them, sua, xoa sinh vien
- Log thoi gian thao tac
- Xem log trong chuong trinh

### 2.5 Phan quyen
- ADMIN: toan quyen
- USER: chi duoc xem, tim kiem, sap xep

## 3. Cau truc thu muc

```text
project1
├── app
│   ├── include
│   │   ├── auth.h
│   │   ├── common.h
│   │   ├── driver_comm.h
│   │   ├── driver_protocol.h
│   │   ├── file_io.h
│   │   ├── logger.h
│   │   └── student.h
│   └── src
│       ├── auth.c
│       ├── driver_comm.c
│       ├── file_io.c
│       ├── logger.c
│       ├── main.c
│       └── student.c
├── data
│   ├── activity.log
│   ├── students.csv
│   └── users.txt
├── driver
│   ├── driver_protocol.h
│   ├── Makefile
│   └── student_driver.c
├── Makefile
└── README.md

Trinh tu chay 
cd ~/project1/driver
make
sudo rmmod student_driver 2>/dev/null
sudo insmod student_driver.ko
sudo chmod 666 /dev/student_driver

cd ~/project1
make
./student_app