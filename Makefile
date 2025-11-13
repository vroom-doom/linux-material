# Kernel module part
obj-m += button_sim.o

# Userspace programs
CC = gcc
APPS = listener trigger

all: modules $(APPS)

modules:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules

# Rule to build userspace programs
$(APPS): %: %.c button_sim.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean
	rm -f $(APPS)