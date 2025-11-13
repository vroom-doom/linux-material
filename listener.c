#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    char buf[1];

    printf("Listener: Opening /dev/button_sim...\n");
    fd = open("/dev/button_sim", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    printf("Listener: Waiting for button press (blocking on read()...)\n");
    
    // This read() call will block until the module wakes it up
    read(fd, buf, 1);

    printf("Listener: Woke up! Event received. Read: '%c'\n", buf[0]);

    close(fd);
    return 0;
}