#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	shmem_init();
	int me = shmem_my_pe();
	srand(me);
	int *counter = shmem_calloc(1, sizeof(*counter));
	for (int i = 0; i < 100; i++)
		if (rand() % (me + 1) == 0)
			shmem_atomic_inc(counter, 0);
	if (me == 0)
		printf("counter is now %d\n", *counter);
	shmem_free(counter);
	shmem_finalize();
	return 0;
}
