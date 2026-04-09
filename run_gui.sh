#!/bin/bash
set -e

PROJECT_DIR="$HOME/project1"
DRIVER_DIR="$PROJECT_DIR/driver"

echo "==> Build driver"
cd "$DRIVER_DIR"
make

echo "==> Reload driver"
sudo rmmod student_driver 2>/dev/null || true
sudo insmod student_driver.ko
sudo chmod 666 /dev/student_driver

echo "==> Build backend"
cd "$PROJECT_DIR"
make

echo "==> Run GUI"
python3 gui/main.py

