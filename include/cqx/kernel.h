/*
 * 'kernel.h' contains some often-used function prototypes etc
 */
void verify_area(void * addr,int count);
void panic(const char * str);

//!Print a string to the console when in User-mode
int printf(const char * fmt, ...);

//!Print a string to the console when in Kernel-mode
int printk(const char * fmt, ...);

//!Write to the TTY device
int tty_write(unsigned ch,char * buf,int count);
