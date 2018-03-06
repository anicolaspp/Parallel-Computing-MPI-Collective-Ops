CC = mpicc


SRC = main.c collective_add.c collective_collect_all.c
HDR = collective_add.h collective_collect_all.h
OBJ = $(SRC:.c=.o)

all:	hw2

hw2:	$(OBJ)
	@echo LINK $(OBJ) INTO $@
	$(CC) $(OBJ) -o $@

clean:
	rm -f *.o *~ hw2
