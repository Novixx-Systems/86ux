#include <cqx/kernel.h>
#include <cqx/autoconf.h>
/*!
 * This function is used through-out the kernel (including mm and fs)
 * to indicate a major problem.
 */
void panic(const char * s)
{
#ifdef CONFIG_PANICS
	printk("Kernel panic: %s\n\rSystem halted.\n\r",s);
	for(;;);
#else
	return;
#endif
}
