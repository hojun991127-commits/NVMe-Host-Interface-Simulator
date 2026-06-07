CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
INCLUDES = -I./include
SRCS = src/main.c src/host.c src/controller.c
OBJS = $(SRCS:.c=.o)
TARGET = nvme_sim

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
		rm -f src/*.o $(TARGET)