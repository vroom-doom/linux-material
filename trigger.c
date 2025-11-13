#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "button_sim.h" // Include the shared header

int main()
{
    int fd;

    printf("Trigger: Opening /dev/button_sim...\n");
    fd = open("/dev/button_sim", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    printf("Trigger: Sending IOCTL command to simulate button press...\n");
    
    // This ioctl() call triggers the "handler" in the kernel
    ioctl(fd, BUTTON_SIM_IOCTL_TRIGGER);

    printf("Trigger: Event sent.\n");

    close(fd);
    return 0;
}