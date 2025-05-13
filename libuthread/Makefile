CC      := gcc
AR      := ar
CFLAGS  := -Wall -Wextra -Werror -g
INC     := -I.

lib := libuthread.a
objs := queue.o uthread.o context.o sem.o preempt.o

all: $(lib)

$(lib): $(objs)
	$(AR) rcs $@ $^

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

uthread.o: uthread.c uthread.h private.h queue.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

context.o: context.c uthread.h private.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

sem.o: sem.c sem.h private.h queue.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

preempt.o: preempt.c private.h uthread.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(lib) *.o