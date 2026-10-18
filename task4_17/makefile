INCDIR = ./include
CC = gcc
CFLAGS = -O2
CFLAGS += -I$(INCDIR)

OBJDIR = obj

_DEPS = task4_prime.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJS = task4_threads.o task4_prime.o xtea.o ipc.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

task4: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) -pthread -lm

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o task4
