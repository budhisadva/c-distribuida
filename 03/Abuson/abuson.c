#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?

#define MASTER 0

/**
 * Inicializa la funcion random
 */
void init_rand() {
  time_t t;
	srand(time(&t));
}

int main(int argc, char* argv[]) {
  int rank, n;
  init_rand();
  MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    // Invocacion de la funcion principal
  MPI_Finalize();
  return 0;
}
