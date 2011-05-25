#include <stdio.h>
#include <syscall.h>


int
main (int argc, char *argv[])
{
	int count;

	for (count = 0; count < 1000; count++)
	{
	float a[64][64], b[64][64], c[64][64];
	float sum;
	int i, j, k, count;

	//init을 밖에서
	for (i = 0; i < 64; i++)
		for (j = 0; j < 64; j++)
			a[i][j] = b[i][j] = 1.0F;



		for (i = 0; i < 64; i++)
			for (j = 0; j < 64; j++)
			{
				sum = 0.0F;
				for (k = 0; k < 64; k=k+8){
					//loop unrolling
					sum += a[i][k] * b[k][j];
					sum += a[i][k+1] * b[k+1][j];
					sum += a[i][k+2] * b[k+2][j];
					sum += a[i][k+3] * b[k+3][j];
					sum += a[i][k+4] * b[k+4][j];
					sum += a[i][k+5] * b[k+5][j];
					sum += a[i][k+6] * b[k+6][j];
					sum += a[i][k+7] * b[k+7][j];
				}
				c[i][j] = sum;
			}
	}


		
	return EXIT_SUCCESS;
}

