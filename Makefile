CC = gcc
CFLAGS = -Wall -Wextra -Iapp/include

COMMON_SRC = app/src/auth.c \
             app/src/student.c \
             app/src/file_io.c \
             app/src/logger.c \
             app/src/driver_comm.c

APP_SRC = app/src/main.c \
          $(COMMON_SRC) \
          app/src/ui.c

BACKEND_SRC = app/src/cli_main.c \
              $(COMMON_SRC)

APP_OUT = student_app
BACKEND_OUT = student_backend

all: $(APP_OUT) $(BACKEND_OUT)

$(APP_OUT): $(APP_SRC)
	$(CC) $(CFLAGS) -o $(APP_OUT) $(APP_SRC) -lncurses

$(BACKEND_OUT): $(BACKEND_SRC)
	$(CC) $(CFLAGS) -o $(BACKEND_OUT) $(BACKEND_SRC)

clean:
	rm -f $(APP_OUT) $(BACKEND_OUT)