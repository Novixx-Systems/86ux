#define __LIBRARY__
#include <unistd.h>
#include <time.h>

static inline _syscall0(int,fork)
static inline _syscall0(int,pause)
static inline _syscall0(int,setup)
static inline _syscall0(int,sync)

#include <cqx/tty.h>
#include <cqx/sched.h>
#include <cqx/head.h>
#include <asm/system.h>
#include <asm/io.h>

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <cqx/fs.h>
#include <cqx/autoconf.h>

static char printbuf[1024];

extern int vsprintf();
extern void init(void);
extern void hd_init(void);
extern long kernel_mktime(struct tm * tm);
extern long startup_time;

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */

#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static void time_init(void)
{
	struct tm time;

	do {
		time.tm_sec = CMOS_READ(0);
		time.tm_min = CMOS_READ(2);
		time.tm_hour = CMOS_READ(4);
		time.tm_mday = CMOS_READ(7);
		time.tm_mon = CMOS_READ(8)-1;
		time.tm_year = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	startup_time = kernel_mktime(&time);
}

static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

int main(void)		/*!The startup routine assumes (well, ...) this */
{
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
	time_init();
	tty_init();
	trap_init();
	sched_init();
	buffer_init();
	hd_init();
	sti();
#ifdef CONFIG_BOOT_PANIC
	panic("Boot panic");
#endif
	move_to_user_mode();
	
	printf("Welcome!\n\n");
	if (!fork()) {		/*!we count on this going ok */
		init();
	}

	for(;;) pause();

	return 0;
}

static char * argv[] = { "/bin/sh",NULL };
static char * envp[] = { "HOME=/root","PATH=/bin","PWD=/", NULL }; // We start at the root folder

void init(void)
{
	int i;
	setup();
	(void) open("/dev/tty0",O_RDWR,0);
	(void) dup(0);
	(void) dup(0);

	

	printf("Loading shell...\n\r");
	if ((i=fork())<0)
		printf("Fork failed\r\n");
	else if (!i) {
		close(0);close(1);close(2);
		setsid();
		(void) open("/dev/tty0",O_RDWR,0);
		(void) dup(0);
		(void) dup(0);
		
		_exit(execve("/bin/sh",argv,envp));
	}
	wait(&i);
	sync();
	init();
}
