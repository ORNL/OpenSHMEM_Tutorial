#include <shmem.h>
#include <stdio.h>

int main(int argc, char **argv) {
	shmem_init();
	int me = shmem_my_pe();
	int npes = shmem_n_pes();
	printf("hello from pe %d of %d\n", me, npes);
	shmem_finalize();
	return 0;
}
