#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define TAG_TEMPORAL 0
#define TAG_DIRECT 2
#define MASTER 0

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
    // Inicializacion de variables
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // set datos
    int datos = rank+20;
    int siguiente = (rank+1)%size;
    int anterior = rank-1;
    if (anterior < 0) {
      anterior = size-1;
    }

    // Comunicacion
    MPI_Send(&datos, 1, MPI_INT, siguiente, TAG_DIRECT, MPI_COMM_WORLD);
    int datosRecibidos;
    MPI_Recv(&datosRecibidos, 1, MPI_INT, anterior, TAG_DIRECT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Soy nodo: %i, recibi: %i, de: %i. \n", rank, datosRecibidos, anterior);
    // if (rank == 0) {
    //   datos = 404;
    // }
    // if (rank == 3) {
    //   MPI_Recv(&datos, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //   printf("Nodo: %i, recibi: %i, de Master \n", rank, datos);
    // }
  MPI_Finalize();
  return 0;
}
