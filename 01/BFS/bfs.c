#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?

#define PROBA 0.5
#define MASTER 0

int conectado(int u, int v){
  if (u+1 == v) return 1;
  srand(time(NULL)*u + v);
  double p = (double)rand()/RAND_MAX;
  return p < PROBA;
}

void bfs(int rank, int size, int target) {
  int visitado[size];
  int mensaje[size+1];    //El ultimo espacio es una bandera
  if (rank == 0) {
    for (int i = 0; i < size; i++) {
      visitado[i] = 0;
    }
    memcpy(mensaje, visitado, size*sizeof(int));
  }
  MPI_Bcast(&visitado, size, MPI_INT, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&mensaje, size+1, MPI_INT, MASTER, MPI_COMM_WORLD);
  visitado[rank] = 1;
  if (rank == 0) {
    printf("> El nodo maestro inicia la busqueda de %d\n", target);
    for (int i = 0; i < size; i++) {
      if (conectado(rank, i)) {
        MPI_Send(mensaje, size+1, MPI_INT, i, 0, MPI_COMM_WORLD);
        printf("El nodo maestro envio mensaje al nodo %i\n", i);
      }
    }
  } else {
    MPI_Recv(mensaje, size+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Nodo actual %d\n", rank);
    if (mensaje[size] == 1) {
      MPI_Finalize();
      exit(0);
    }
    visitado[rank] = 1;
    if (rank == target) {
      mensaje[size] = 1;
      printf("> Nodo objeto ha sido encontrado por %i\n", rank);
      MPI_Finalize();
      exit(0);
    }
    for (int i = 0; i < size; i++) {
      if (conectado(rank, i)) {
        MPI_Send(mensaje, size+1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(visitado, size, MPI_INT, i, 0, MPI_COMM_WORLD);
        printf("el nodo %i envia mensaje al nodo %i \n", rank, i);
      }
    }
  }
}

int main(int argc, char* argv[]) {
  int rank, size, target;
  MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // El nodo maestro escoge de forma aleatoria el nodo a buscar
    if (rank == 0) {
      srand(time(NULL));
      target = rand() % (size-1)+1;
    }
    MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);
    bfs(rank, size, target);
  MPI_Finalize();
  return 0;
}
