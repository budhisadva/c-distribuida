#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?

#define PROBA 0.5

#define MASTER 0

#define TAG_IGNORE 0
#define TAG_SUCESS 1
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
 * @param n, tamaño de la red
 * @param target, indice del nodo que buscamos
 */
void dfs(int rank, int n, int target) {
  int visitado[n+1];
  int parent[n];
  int profundidad = 0;
  if (rank == 0) {
    setArreglo(visitado, n+1);
    setArreglo(parent, n);
  }
  MPI_Bcast(&visitado, n+1, MPI_INT, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&parent, n, MPI_INT, MASTER, MPI_COMM_WORLD);
  // MASTER --------------------------------
  if (rank == 0) {
    visitado[0] = 1;
    parent[0] = 0;
    printf("> MASTER => Buscar: Nodo %i\n", target);
    // Envia mensaje a un nodo al azar entre 1 y n
    for (int i = 1; i < n; i++) {
      if (conectado(rank, i, n)) {
        printf("> MASTER => enviado a <Nodo %i>\n", i);
        MPI_Send(&visitado, n+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
        MPI_Send(&parent, n, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
        break;
      }
    }
    // Espera de una respuesta exitosa
    MPI_Recv(&visitado, n+1, MPI_INT, MPI_ANY_SOURCE, TAG_SUCESS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("%s\n", "El nodo target ha sido encontrado.");
    //Envia respuesta exitosa a los nodos no visitados
    for (int i = 1; i < n; i++) {
      if (!visitado[i]) {
        MPI_Send(&visitado, n+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
        MPI_Send(&parent, n, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
      }
    }
  }
  // RESTO DE NODOS ----------------------
  else {
    MPI_Status status;
    MPI_Recv(&visitado, n+1, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&parent, n, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, &status);
    // el nodo objetivo no ha sido encontrado
    if (!visitado[n]) {
      // El nodo actual no ha sido visitado
      if (!visitado[rank]) {
        visitado[rank] = 1;
        parent[rank] = status.MPI_SOURCE;
        // el nodo actual es el objetivo
        if (rank == target) {
          visitado[n] = 1;
          printf("> Nodo %i => SOY EL OBJETIVO\n", rank);
          MPI_Send(&visitado, n+1, MPI_INT, parent[rank], TAG_SUCESS, MPI_COMM_WORLD);
          printf("> Nodo %i => enviado a <Nodo %i>\n", rank, parent[rank]);
        } else {
          // Envia mensaje a un nodo al azar entre 1 y n que no haya sido visitado
          for (int i = 1; i < n; i++) {
            if (!visitado[i] && conectado(rank, i, n)) {
              printf("> Nodo %i => enviado a <Nodo %i>\n", rank, i);
              MPI_Send(&visitado, n+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
              MPI_Send(&parent, n, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
              break;
            }
          }
          // quedo a la espera de una respuesta exitosa
          MPI_Recv(&visitado, n+1, MPI_INT, MPI_ANY_SOURCE, TAG_SUCESS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          // envia respuesta exitosa al padre
          MPI_Send(&visitado, n+1, MPI_INT, parent[rank], TAG_SUCESS, MPI_COMM_WORLD);
          printf("> Nodo %i => enviado a <Nodo %i>\n", rank, parent[rank]);
        }
      }
    }
  }
}

int main(int argc, char* argv[]) {
  int rank, n, target;
  init_rand();
  MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    if (rank == 0) {//escogemos nodo a buscar
      target = rand()%(n-1)+1;
    }
    MPI_Bcast(&target, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    dfs(rank, n, target);
  MPI_Finalize();
  return 0;
}
