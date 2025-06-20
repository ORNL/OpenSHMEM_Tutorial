/*
 * heat2d_oshmem.c – Minimal 2-D heat-diffusion (Jacobi stencil) in OpenSHMEM
 *
 * This version prints **useful diagnostics** at the end:
 *   – Total wall-time and average time per iteration
 *   – Global min/avg/max temperature after the final iteration
 *
 * Inspired by Chapel and UPC heat-equation tutorial codes.
 *
 * Compile:
 *     oshcc -O2 -std=c99 -o heat2d heat2d_oshmem.c
 * Run (example with 4 PEs):
 *     oshrun -n 4 ./heat2d
 */

#include <float.h>
#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
 

#ifndef NX
#define NX 256          /* global columns */
#endif

#ifndef NY
#define NY 256          /* global rows */
#endif

#ifndef MAX_ITERS
#define MAX_ITERS 500   /* time steps */
#endif

#ifndef ALPHA
#define ALPHA 0.1       /* diffusion coefficient */
#endif

/* Row-major 2-D indexing macro */
static inline size_t IDX(int i, int j) {
	return (size_t)i * NX + (size_t)j;
}

static double wtime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char **argv) {
	shmem_init();
	printf("shmem version %d.%d\n", SHMEM_MAJOR_VERSION, SHMEM_MINOR_VERSION);
	const int npes = shmem_n_pes();
	const int me   = shmem_my_pe();

	/* --- 1. Block-row decomposition ----------------------------------- */

	int base   = NY / npes;            /* rows per PE (floor) */
	int extra  = NY % npes;            /* first <extra> PEs get +1 row */
	int local_ny = base + (me < extra);
	int start_row = me * base + (me < extra ? me : extra);

	/* +2 ghost rows */

	const int alloc_rows = local_ny + 2;
	double *u  = shmem_malloc((size_t)alloc_rows * NX * sizeof(double));
	double *un = shmem_malloc((size_t)alloc_rows * NX * sizeof(double));

	if (!u || !un) {
		fprintf(stderr, "PE %d: shmem_malloc failed\n", me);
		shmem_global_exit(1);
	}

	/* --- 2. Initial conditions ---------------------------------------- */
	
	for (int i = 0; i < alloc_rows; ++i)
		for (int j = 0; j < NX; ++j)
			u[IDX(i, j)] = 25.0;

	if (start_row == 0)
		for (int j = 0; j < NX; ++j)
			u[IDX(1, j)] = 100.0;      /* top edge */

	if (start_row + local_ny == NY)
		for (int j = 0; j < NX; ++j)
			u[IDX(local_ny, j)] = 0.0; /* bottom edge */

	shmem_barrier_all();
	double t0 = wtime();

	/* --- 3. Time stepping --------------------------------------------- */

	for (int iter = 0; iter < MAX_ITERS; ++iter) {
		/* 3a. Halo exchange (north/south) */
		if (me > 0)
			shmem_putmem(&u[IDX(0, 0)], &u[IDX(1, 0)], NX * sizeof(double), me - 1);

		if (me < npes - 1)
			shmem_putmem(&u[IDX(local_ny + 1, 0)], &u[IDX(local_ny, 0)], NX * sizeof(double), me + 1);

		shmem_barrier_all();

		/* 3b. Jacobi update (interior points) */

		for (int i = 1; i <= local_ny; ++i)
			for (int j = 1; j < NX - 1; ++j)
				un[IDX(i, j)] = u[IDX(i, j)] + ALPHA * (u[IDX(i - 1, j)] + u[IDX(i + 1, j)] + u[IDX(i, j - 1)] + u[IDX(i, j + 1)] - 4.0 * u[IDX(i, j)]);

		shmem_barrier_all();
		double *tmp = u; u = un; un = tmp;
	}

	shmem_barrier_all();
	double t1 = wtime();

	/* --- 4. Diagnostics ------------------------------------------------ */
	/* local min / max / sum (exclude ghost rows) */

	static double l_min = DBL_MAX, l_max = -DBL_MAX, l_sum = 0.0;
	for (int i = 1; i <= local_ny; ++i) {
		for (int j = 0; j < NX; ++j) {
			double v = u[IDX(i, j)];
			if (v < l_min) l_min = v;
			if (v > l_max) l_max = v;
			l_sum += v;
		}
	}

	/* global reductions */

	static double g_min, g_max, g_sum;
	shmem_double_min_reduce(SHMEM_TEAM_WORLD, &g_min, &l_min, 1);
	shmem_double_max_reduce(SHMEM_TEAM_WORLD, &g_max, &l_max, 1);
	shmem_double_sum_reduce(SHMEM_TEAM_WORLD, &g_sum, &l_sum, 1);

	if (me == 0) {
		double g_avg = g_sum / (double)(NX * NY);
		double total = t1 - t0;

		printf("--- Heat-2D OpenSHMEM Report ---\n");
		printf("Grid            : %dx%d\n", NY, NX);
		printf("PEs             : %d\n", npes);
		printf("Iterations      : %d\n", MAX_ITERS);
		printf("Total time (s)  : %.4f\n", total);
		printf("Time/iter (ms)  : %.3f\n", 1e3 * total / MAX_ITERS);
		printf("Final Tmin/Tavg/Tmax : %.2f / %.2f / %.2f °C\n", g_min, g_avg, g_max);
	}

	shmem_free(u);
	shmem_free(un);
	shmem_finalize();
	return 0;
}
