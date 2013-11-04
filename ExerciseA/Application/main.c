// C Program for testing IOCTL call
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include "../common.h"

#define CHAR_DEVICE "/dev/sysled"
#define CALL_ARG 5

int main(void)
{
	printf("Test application started.\n");

	int fd;
	fd = open(CHAR_DEVICE, O_RDWR);	// Opening with READ & WRITE access

	if (fd == -1)
	{
		printf("Failed to access file, ERROR: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}	
	
	// Send an IOCTL call
	if (ioctl(fd, SYSLED_IOC_CALL, CALL_ARG) != -1)
	{
		printf("IOCTL called from application with arg: %d.\n", CALL_ARG);	
	}
	else
	{
		printf("Failed to send IOCTL call, ERROR: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	close(fd);

	return EXIT_SUCCESS;
}
