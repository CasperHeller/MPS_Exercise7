#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include "../common.h"

#define CHAR_DEVICE "/dev/sysled"
#define BUFFER 100

int main()
{
	printf("Test application for timer started.\n");

	int fd;
	fd = open(CHAR_DEVICE, O_RDWR);

    if (fd == -1)
	{
		printf("Failed to access file: %s, ERROR: %s\n", CHAR_DEVICE, strerror(errno));
		return EXIT_FAILURE;
	}	

	char inBuffer[BUFFER];
	int running = 1;
	while(running)
	{
		printf("Do you want to 'start', 'stop', set the 'timeout' or 'exit'?\n");
        while (scanf("%s", inBuffer) != 1);
		
		int i = 0;
		while (inBuffer[i] != '\0')
		{
			inBuffer[i] = tolower(inBuffer[i]);
			++i; 
		}
		
		// Starting timer:
		if ( !strcmp(inBuffer, "start") )
		{
			if (ioctl(fd, SYSLED_IOC_TIMER_START) == -1)
                printf("Failed to send ioctl call, ERROR: %s\n", strerror(errno));
			else
				printf("Timer started.\n");
		}
		// Stopping timer:
		else if ( !strcmp(inBuffer, "stop") )
		{
			if (ioctl(fd, SYSLED_IOC_TIMER_STOP) == -1)
                printf("Failed to send ioctl call, ERROR: %s\n", strerror(errno));
			else
				printf("Timer stopped.\n");
		}
		// Setting frequency:
		else if ( !strcmp(inBuffer, "timeout") )
		{
			printf("What timeout between toggles do you want?\n");
			int freq = 0;
			while (scanf("%d", &freq) != 1);
			if (ioctl(fd, SYSLED_IOC_TIMER_FREQ, freq) == -1)
                printf("Failed to send ioctl call, ERROR: %s\n", strerror(errno));
			else
				printf("Timeout set to %d.\n", freq);
		}
		// Exiting program:
		else if ( !strcmp(inBuffer, "exit") )
		{
			running = 0;
		}
		else
		{
			printf("Unknown command.\n");
		}
	}

	close(fd);
	return EXIT_SUCCESS;
}
