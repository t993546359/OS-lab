#ifndef __lib_h__
#define __lib_h__
#include "types.h"
//typedef unsigned int uint32_t
#define SYS_sleep 21 
#define SYS_fork 20
#define SYS_exit 22
#define SYS_write 4
#define SEM_init 5
#define SEM_post 6
#define SEM_wait 7
#define SEM_des 8
void printf(const char *format,...);
int fork();
int sleep(uint32_t time);
int exit();
#endif
