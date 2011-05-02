#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  int i;

  for (i = 1; i < argc; i++) {
    printf ("executing %s ", argv[i]);
		wait(exec(argv[i]));
		printf ("\n");
	}
  
	printf ("\n");


	
	return EXIT_SUCCESS;
}
