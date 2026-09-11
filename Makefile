NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRC = src/main.c \
	src/parsing/parse.c \
	src/init/init.c \
	src/init/init_helpers.c \
	src/init/init_cleanup.c \
	src/coder/coder.c \
	src/coder/coder_lifecycle.c \
	src/coder/coder_dongles.c \
	src/coder/coder_release.c \
	src/scheduler/scheduler.c \
	src/scheduler/scheduler_requests.c \
	src/scheduler/scheduler_wait.c \
	src/scheduler/scheduler_heap.c \
	src/scheduler/scheduler_heap_utils.c \
	src/monitor/monitor.c \
	src/monitor/monitor_check.c \
	src/utils/utils.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
