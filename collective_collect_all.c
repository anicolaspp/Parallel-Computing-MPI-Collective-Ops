//
// Created by Nicolas A Perez on 2/11/18.
//

#include "collective_collect_all.h"
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>


int number_of_dimensions(int numberOfProcessors);
int get_adj(int processorId, int dimension);
int total_package_size(const int *count_buffer);

void encode_package(char *const *recvbuf, const int *recvcnt, char *package);
void update_recvcnt(int *recvcnt, const int *receiving_buffer);
void decode_package(char **recvbuf_internal, const int *recvcnt_internal, const char *incoming_package);
void update_output(char **recvbuf, int *recvcnt, char *const *recvbuf_internal, const int *recvcnt_internal);
void init_buffers(char *sendbuf, int sendcnt, char ***recvbuf_internal, int **recvcnt_internal);

int p, id;

/*
 * collect_all uses a hypercube algorithm in the following way
 * 1 - On each dimension, each processor identify another processor to exchange data
 * 2 - Two processors exchange data (all available data stored in recvbuf_internal) in one step by using encode_package
 * 3 - decode_package is then used to transform an (int*) into (int**)
 * 4 - Once the process has occurred for each dimension, then recvcnt_internal holds the data of every single processor
 * 5 - recvbuf_internal is copied into recvbuf so it can be used as output parameter.
 */
void collect_all(char *sendbuf, int sendcnt, char **recvbuf, int *recvcnt) {
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    char **recvbuf_internal;
    int *recvcnt_internal;

    init_buffers(sendbuf, sendcnt, &recvbuf_internal, &recvcnt_internal);

    int dimensions = number_of_dimensions(p);

    for (int i = 0; i < dimensions; ++i) {
        int adj = get_adj(id, i);

        int receiving_buffer[p];

        MPI_Sendrecv(recvcnt_internal, p, MPI_INT, adj, 0, receiving_buffer, p, MPI_INT, adj, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

        // total number of items to be sent
        int total_size_to_send = total_package_size(recvcnt_internal);

        //create buffer to package the data to be sent
        char *out_package = calloc((size_t) total_size_to_send, sizeof(char));

        //in here we flatten the recvbuf_internal into a 1-D array so we can send all data at once
        encode_package(recvbuf_internal, recvcnt_internal, out_package);

        //the size to be received is based on receiving_buffer
        int total_size_to_receive = total_package_size(receiving_buffer);

        //we need a package with the correct size to receive the incoming message
        char *in_package = calloc((size_t) total_size_to_receive, sizeof(char));

        MPI_Sendrecv(out_package, total_size_to_send, MPI_CHAR, adj, 0, in_package, total_size_to_receive, MPI_CHAR,
                     adj, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        update_recvcnt(recvcnt_internal, receiving_buffer);

        //in here we transformed a 1-D array into the 2-D recvbuf_internal
        decode_package(recvbuf_internal, recvcnt_internal, in_package);

        free(out_package);
        free(in_package);
    }

    update_output(recvbuf, recvcnt, recvbuf_internal, recvcnt_internal);
}

void init_buffers(char *sendbuf, int sendcnt, char ***recvbuf_internal, int **recvcnt_internal) {
    (*recvbuf_internal) = calloc((size_t) p, sizeof(char *));
    (*recvcnt_internal) = calloc((size_t) p, sizeof(int));

    (*recvbuf_internal)[id] = sendbuf;

    if (sendbuf == NULL) {
        (*recvcnt_internal)[id] = 0;
    } else {
        (*recvcnt_internal)[id] = sendcnt;
    }
}

void update_output(char **recvbuf, int *recvcnt, char *const *recvbuf_internal, const int *recvcnt_internal) {
    *recvbuf = (char *) recvbuf_internal;
    *recvcnt = (int) calloc((size_t) p, sizeof(int));

    for (int i = 0; i < p; i++) {
        recvcnt[i] = recvcnt_internal[i];
    }
}

void decode_package(char **recvbuf_internal, const int *recvcnt_internal, const char *incoming_package) {
    int c = 0;

    for (int i = 0; i < p; i++) {

        if (recvbuf_internal[i] == NULL && recvcnt_internal[i] > 0) {
            recvbuf_internal[i] = calloc((size_t) recvcnt_internal[i], sizeof(char));

            for (int j = 0; j < recvcnt_internal[i]; j++) {
                recvbuf_internal[i][j] = incoming_package[c++];
            }
        }
    }
}

void update_recvcnt(int *recvcnt, const int *receiving_buffer) {
    for (int j = 0; j < p; ++j) {
        if (j != id && recvcnt[j] == 0) {
            recvcnt[j] = receiving_buffer[j];
        }
    }
}

void encode_package(char *const *recvbuf, const int *recvcnt, char *package) {
    int c = 0;

    for (int i = 0; i < p; i++) {
        int count = recvcnt[i];

        for (int k = 0; k < count; k++) {
            package[c++] = recvbuf[i][k];
        }
    }
}

int total_package_size(const int *count_buffer) {
    int sum = 0;

    for (int i = 0; i < p; ++i) {
        sum += count_buffer[i];
    }

    return sum;
}

int number_of_dimensions(int numberOfProcessors) {
    return (int) log2(numberOfProcessors);
}

// it finds the corresponding processor to communicate from processorId based on the current dimension
int get_adj(int processorId, int dimension) {
    return processorId ^ (int) pow(2, dimension);
}