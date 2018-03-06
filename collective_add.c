//
// Created by Nicolas A Perez on 2/11/18.
//

#include "collective_add.h"
#include <mpi.h>

int indexOf(int value, const int processors[], int len);
int *swap(int px, int py, int processors[]);
int reduce_processors(int processors[], int len);
void init_processors(int root, int *processors, int len);
void reduce_last_processor_if_needed(const int *processes, int *partial_sum, int *len);
void exchange_partial_sums(int *processes, int index, int *partial_sum, int *len);

int p, id;

/*
 * implementation of collective operation using binomial tree.
 * The function does message exchange between processor in odd and even positions at once.
 * Then it re-arranges the processor since the ones on odd positions will be done at this point,
 * we then repeat the process.
 * If at some point there is an odd number of processor, the last one exchange with the first one
 * and then the last one gets eliminated which implies we have now an even number of processor.
 *
 * Calling init_processors put the root processor at the first position so that the final sum ends always
 * on the root processor.
 * */
int add_all(int value, int root) {
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    int processors[p];
    int partial_sum = value;
    int len = p;

    init_processors(root, processors, len);

    for (int i = 0; i < p / 2; ++i) {

        reduce_last_processor_if_needed(processors, &partial_sum, &len);

        int index = indexOf(id, processors, len);

        exchange_partial_sums(processors, index, &partial_sum, &len);
    }

    if (id == root) {
        return partial_sum;
    } else {
        return 0;
    }
}

void init_processors(int root, int *processors, int len) {
    for (int i = 0; i < len; ++i) {
        processors[i] = i;
    }

    int indexOfRoot = indexOf(root, processors, len);

    swap(0, indexOfRoot, processors);
}

int indexOf(int value, const int processors[], int len) {
    for (int i = 0; i < len; ++i) {
        if (processors[i] == value) {
            return i;
        }
    }

    return -1;
}

void exchange_partial_sums(int *processes, int index, int *partial_sum, int *len) {
    if (index >= 0) {

        // if my position is even, then I will receive from the next one
        if (index % 2 == 0) {
            int sum_received;
            MPI_Recv(&sum_received, 1, MPI_INT, processes[index + 1], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            (*partial_sum) += sum_received;
        } else { // if my position is odd, then I will send to previous one
            MPI_Send(partial_sum, 1, MPI_INT, processes[index - 1], 0, MPI_COMM_WORLD);
        }

        // reduce_processor takes out all odd processors after the exchange.
        (*len) = reduce_processors(processes, (*len));
    }
}

void reduce_last_processor_if_needed(const int *processes, int *partial_sum, int *len) {

    // If the number of processors is odd, then we do partial_sum exchange between the last one and the first one (root)
    if ((*len) % 2 != 0) {
        if (id == processes[(*len) - 1]) {
            MPI_Send(partial_sum, 1, MPI_INT, processes[0], 0, MPI_COMM_WORLD);
        } else if (id == processes[0]) {
            int sum_received;
            MPI_Recv(&sum_received, 1, MPI_INT, processes[(*len) - 1], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            (*partial_sum) += sum_received;
        }

        // by reducing the length we have taken the last processor out after the exchange.
        (*len) -= 1;
    }
}

int reduce_processors(int processors[], int len) {
    int i = 0;

    for (int j = 0; j < len; j += 2) {
        processors[i++] = processors[j];
    }

    return len / 2;
}

int *swap(int px, int py, int processors[]) {
    int v = processors[px];
    processors[px] = py;
    processors[py] = v;

    return processors;
}
