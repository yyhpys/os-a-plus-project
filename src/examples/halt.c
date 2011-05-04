/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the operating system. */

#include <syscall.h>

int
main (void)
{
  for(;;){

			printf("wow!");
	}
  /* not reached */
}
