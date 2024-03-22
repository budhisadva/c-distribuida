#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?

#define PROBA 0.5

#define MASTER 0

#define TAG_IGNORE 0
#define TAG_FOUND 1
#define TAG_NOT_FOUND 2
/**
 *
 */
/**
 * Inicializa la funcion random
 */
void init_rand() {
  time_t t;
	srand(time(&t));
}

/**
 * Función que determina si dos nodos están conectados
 * @param u, indice del nodo de referencia
 * @param v, indice del nodo a comprobar
 * @param size,tamaño de la red
 * return 1 si p > 0.5 o si v es el ultimo nodo
 *        0 en otro caso
 */
int conectado(int u, int v, int size){
  if (u == v) return 0;
  if (v == size-1) return 1;
  double p = (double)rand()/RAND_MAX;
  return p > PROBA;
}

/**
 * Garatiza los valores iniciales del arreglo
 */
 void setArreglo(int *arr, int tam){
   for (int i = 0; i < tam; i++) {
     arr[i] = 0;
   }
}

/**
 * dfs
 * @param rank, indice del nodo que manda a llamar
 * @param size, tamaño de la red
 * @param target, indice del nodo que buscamos
 */
void dfs(int rank, int size, int target) {
  int visitado[size+1];
  int parent[size];
  if (rank == 0) {
    setArreglo(visitado, size+1);
    setArreglo(parent, size);
  }
  MPI_Bcast(&visitado, size+1, MPI_INT, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&parent, size, MPI_INT, MASTER, MPI_COMM_WORLD);
  // MASTER --------------------------------
  if (rank == 0) {
    visitado[rank] = 1;
    parent[rank] = 0;
    printf("> MASTER => Buscar: Nodo %i\n", target);
    for (int i = 1; i < size; i++) {
      if (conectado(rank, i, size)) {
        printf("> MASTER => enviado a <Nodo %i>\n", i);
        MPI_Send(&visitado, size+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
        MPI_Send(&parent, size, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
        break;
      }
    }
    MPI_Recv(&visitado, size+1, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&parent, size, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("%s\n", "El nodo target ha sido encontrado.");
    for (int i = 1; i < size; i++) {
      MPI_Send(&visitado, size+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
      MPI_Send(&parent, size, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
    }
  }
  // RESTO DE NODOS ----------------------
  else {
    MPI_Status status;
    MPI_Recv(&visitado, size+1, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&parent, size, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, &status);
    if (visitado[size] != 1) {
      // Buscamos el nodo objetivo
      if (visitado[rank] != 1) {
        // El nodo actual no ha sido visitado
        visitado[rank] = 1;
        parent[rank] = status.MPI_SOURCE;
        if (rank == target) {
          // el nodo actual es el objetivo
          visitado[size] = 1;
          printf("> Nodo %i => SOY EL OBJETIVO\n", rank);
          MPI_Send(&visitado, size+1, MPI_INT, parent[rank], TAG_IGNORE, MPI_COMM_WORLD);
          MPI_Send(&parent, size, MPI_INT, parent[rank], TAG_IGNORE, MPI_COMM_WORLD);
          printf("> Nodo %i => enviado a <Nodo %i>\n", rank, parent[rank]);
        } else {
          // el nodo actual no es el objetivo
          for (int i = 1; i < size; i++) {
            if (visitado[i] != 1 && conectado(rank, i, size)) {
              // Buscamos un nodo conectado y sin visitar
              printf("> Nodo %i => enviado a <Nodo %i>\n", rank, i);
              MPI_Send(&visitado, size+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
              MPI_Send(&parent, size, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
              break;
            }
          }
        }
      }
    } else {
      // el objetivo ha sido encontrado
      MPI_Send(&visitado, size+1, MPI_INT, parent[rank], TAG_IGNORE, MPI_COMM_WORLD);
      MPI_Send(&parent, size, MPI_INT, parent[rank], TAG_IGNORE, MPI_COMM_WORLD);
      // printf("> Nodo %i => enviado a <Nodo %i>\n", rank, parent[rank]);
    }
  }
}

int main(int argc, char* argv[]) {
  int rank, size, target;
  init_rand();
  MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {//escogemos nodo a buscar
      target = rand()%(size-1)+1;
    }
    MPI_Bcast(&target, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    dfs(rank, size, target);
  MPI_Finalize();
  return 0;
}
