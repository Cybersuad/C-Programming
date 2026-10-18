INCDIR = ./include
CC = gcc
CFLAGS = -O2
CFLAGS += -I$(INCDIR)

OBJDIR = obj

_DEPS =
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJS = task2.o task2_abun.o task2_cube.o task2_def.o task2_fib.o task2_kvad.o task2_odd.o task2_perf.o task2_prim.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

task2: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) -lm

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o task2
