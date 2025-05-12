CC      := gcc
AR      := ar
CFLAGS  := -Wall -Wextra -Werror -g
INC     := -I.

lib := libuthread.a
objs := queue.o uthread.o context.o sem.o preempt.o

all: $(lib)

$(lib): $(objs)
	$(AR) rcs $@ $^

clean:
	rm -f $(lib) *.o