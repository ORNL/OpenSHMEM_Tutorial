#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	shmem_init();
	int *symmetric = shmem_malloc(sizeof(symmetric[0]) * 8);
	int *local = malloc(sizeof(local[0]) * 4);
	printf("local %s remotely accessible, and symmetric %s\n", shmem_addr_accessible(local, 0) ? "is" : "is not", shmem_addr_accessible(symmetric, 0) ? "is" : "is not");
	symmetric = shmem_realloc(symmetric, sizeof(symmetric[0]) * 16);
	free(local);
	shmem_free(symmetric);
	shmem_finalize();
	return 0;
}
