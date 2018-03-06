#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// my function headers
#include "collective_add.h"
#include "collective_collect_all.h"

int p, id;

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    int root = 0;

    int sum = add_all(id, root);

    if (id == root) {
        printf("[%d] add_all->%d\n", root, sum);
    }

    char *buff = calloc(18, sizeof(char));

    sprintf(buff, "hello from rank %d", id);
    int size = strlen(buff) + 1;

    char **recvbuf = NULL;
    int recvcnt_me[p];

    collect_all(buff, size, (char **) &recvbuf, (int *) &recvcnt_me);

    if(id == 2) {
        printf("[2] collect_all[4]->\"%s\" (sz=%d)\n", recvbuf[3], recvcnt_me[3]);
    }

    MPI_Finalize();

    return 0;
}