#pragma once
#include <linux/ioctl.h>


#define SYSLED_IOC_MAGIC 'k'
#define SYSLED_IOC_TIMER_START 	_IO(SYSLED_IOC_MAGIC, 0)
#define SYSLED_IOC_TIMER_STOP  	_IO(SYSLED_IOC_MAGIC, 1)
#define SYSLED_IOC_TIMER_FREQ 	_IOW(SYSLED_IOC_MAGIC, 2, int)
