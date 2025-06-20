#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	shmem_init();
	int me = shmem_my_pe();
	int npes = shmem_n_pes();
	shmem_team_t team;
	shmem_team_config_t *config = NULL;
	shmem_team_split_strided(SHMEM_TEAM_WORLD, 0, 2, npes / 2, config, 0, &team);
	if (team != SHMEM_TEAM_INVALID) {
		int team_id = shmem_team_my_pe(team);
		int team_size = shmem_team_n_pes(team);
		printf("hello from %d of team, my world ID is %d!\n", team_id, me);
	}
	shmem_team_destroy(team);
	shmem_finalize();
	return 0;
}
