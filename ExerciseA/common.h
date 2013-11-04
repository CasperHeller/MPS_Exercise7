#pragma once
#include <linux/ioctl.h>


#define SYSLED_IOC_MAGIC 'k'
#define SYSLED_IOC_CALL _IOW(SYSLED_IOC_MAGIC, 0, int)
