INCDIR = ./include
CC = gcc
CFLAGS = -O2
CFLAGS += -I$(INCDIR)

OBJDIR = obj

_DEPS = ewpdef.h network.h 
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJS = task5.o network.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

task5: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) -lm

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o task5
