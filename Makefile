CC = gcc
CFLAGS = -Wall -Wextra -Iapp/include

APP_SRC = app/src/main.c \
          app/src/auth.c \
          app/src/student.c \
          app/src/file_io.c \
          app/src/logger.c \
          app/src/driver_comm.c

APP_OUT = student_app

all: $(APP_OUT)

$(APP_OUT): $(APP_SRC)
	$(CC) $(CFLAGS) -o $(APP_OUT) $(APP_SRC)

clean:
	rm -f $(APP_OUT)