#ifndef BUTTON_SIM_H
#define BUTTON_SIM_H

#include <linux/ioctl.h>

// Define the 'magic' number for our ioctl commands. 'k' is a common choice.
#define BUTTON_SIM_MAGIC 'k'

// Define our one and only ioctl command.
// _IO means it's a simple command with no arguments.
#define BUTTON_SIM_IOCTL_TRIGGER _IO(BUTTON_SIM_MAGIC, 1)

#endif // BUTTON_SIM_H