# Comiler and Linker
CC = g++
LD = g++

CFLAGS=
#CFLAGS += -pedantic
#CFLAGS += -ansi
#CFLAGS += -Wno-long-long
CFLAGS += -O2
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wconversion

LDFLAGS =
LDFLAGS += -lrt
LDFLAGS += -lpthread

OBJS += main.o
OBJS += time_unit.o
OBJS += time_period.o
OBJS += rt_task.o

# .h files that are not part of any .cpp file
# maybe a better way to do this?
H_FILES =
H_FILES += data_types.h

PROG = ntp

# ********************** all ****************************
all: $(PROG)

ntp: ntp.o time_unit.o
	$(LD) $(LDFLAGS) ntp.o time_unit.o -o $@

%.o: %.cpp %.h $(H_FILES)
	$(CC) $(CFLAGS) -c $<

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f $(PROG)
	rm -f .*.swp
	rm -f *~
	rm -f fifo.out sporadic.out
	rm -f ntp

run: $(PROG)
	./$(PROG)
