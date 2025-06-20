#include <inttypes.h>
#include <math.h>
#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 100000

double circle(double x) {
	return 1 - pow(x, 2);
}

int main(){
	shmem_init();
	int me = shmem_my_pe();
	int npes = shmem_n_pes();

	static long count = 0;
	double f_x;

	//seed the randomizer
	srand(time(0) + me);
	for (int i = 0; i < NUM; i++) {
		//generate point(x, y) in first quadrant
		//note: double is a 64 bit floating point
		double x = (double) rand() / (double) RAND_MAX;
		double y = (double) rand() / (double) RAND_MAX;
		f_x = circle(x);
		if (pow(y, 2) <= f_x)
			count += 1;
	}

	printf("%d: count %ld\n", me, count);

	//needs to be in symmetric memory
	static long total = 0;

	//note that we are only reducing one element
	shmem_long_sum_reduce(SHMEM_TEAM_WORLD, &total, &count, 1);   

	if (me == 0) {
		printf("%d: count total: %ld\n", me, total);
		printf("%d: ratio: %f\n", me, (double)total / ((double)NUM * npes));
		printf("%d: est of pi: %f\n", me, (double)total / ((double)NUM * npes) * 4);
	}

	return 0;
}
