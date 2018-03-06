# Parallel-Computing-MIP-Collective-Ops
Implementation of MPI Collective Operations from the scratch

# Implementing Collective Operations

# Objectives:
This goal of this assignment is to practice MPI programming and to design collective operations by hand (using point-to-point MPI operations).

# Details:
You are asked to implement two functions: ***add_all()*** and ***collect_all()*** :

The following function is a collective operation; that is, it shall be called by all mpi processes or none at all. (Use ***MPI_COMM_WORLD***.) 
Each process is expected to provide an integer 'x', and the function returns the total sum for the process whose rank is ***root***. The return value of all other processes is zero. 

The function signature should be:
```
int add_all(int x, int root);
```

The following function is a collective operation; that is, it shall be called by all mpi processes or none at all. (Use MPI_COMM_WORLD.) 

Each process is expected to provide a block of data of size ***sendcnt*** stored in ***sendbuf***. 

When the function returns, each process will have the data blocks sent from all other processes (including itself) stored in ***recvbuf***. ***recvbuf*** will be an array of pointers to the received data (the function is in charge of allocating the array and the buffer for the received data). ***recvcnt*** is an array storing the size of the data. 

Again, the function is responsible for allocating the array. More specifically, the data from the i-th process is received by every process and placed in the i-th element of the ***recvbuf*** and its size is put in the i-th element of ***recvcnt***. One must not assume that all processes would provide the same ***sendcnt***. 

```
void collect_all(char* sendbuf, int sendcnt, char** recvbuf, int* recvcnt);
```

The following is a test program (you can make more complicated tests):

```
...
int p, id;
MPI_Comm_size(MPI_COMM_WORLD, &p);
MPI_Comm_rank(MPI_COMM_WORLD, &id);

int sum = add_all(id, 0);
if(id==0) printf("[0] add_all->%d\n", sum);

char str[100];
sprintf(str, "hello from rank %d", id);
char**rbuf;
int* rcnt;
collect_all(ch, strlen(str)+1, &rbuf, &rcnt)
if(id==2) print("[2] collect_all[3]->\"%s\" (sz=%d)\n", rbuf[3], rcnt[3]);
...
```

If you run the test with, say, 4 ***MPI*** processes, the result should look similar to:

```
[0] add_all->6
[2] collect_all[3]->"hello from rank 3" (sz=18) 
```

To implement the two functions above, you are only allowed to use ***MPI*** point-to-point communication routines (***MPI_Send***, ***MPI_Recv***, ***MPI_Isend***, ***MPI_Irecv***, ***MPI_Sendrecv***, etc.). In particular, you are ***NOT*** allowed to use ***MPI collective functions for the implementation***. 

# IMPORTANT:

For ***add_all()***, you should use the binomial tree. For ***collect_all()***, you should use the hypercube. 
You may assume that the number of ***MPI*** processes must be a power of two for ***collect_all()***.
For ***add_all()***, there can be an arbitrary number of ***MP***I processes (not necessarily power of two).
