#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?

#define PROBA 0.5
#define MASTER 0
#define TAG_IGNORE 0
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
 * return 1 si p > 0.5 o si v es el siguiente inmediato de u
 *        0 en otro caso
 */
int conectado(int u, int v){
  if (u+1 == v) return 1;
  double p = (double)rand()/RAND_MAX;
  return p > PROBA;
}

/**
 * bfs
 * @param rank, indice del nodo que manda a llamar
 * @param size, tamaño de la red
 * @param target, indice del nodo que buscamos
 */
void bfs(int rank, int size, int target) {
  int visitado[size+1];//El ultimo espacio es una bandera
  if (rank == 0) {
    for (int i = 0; i < size+1; i++) {
      visitado[i] = 0;
    }
  }
  // Marcar nodo actual como visitado
  MPI_Bcast(&visitado, size, MPI_INT, MASTER, MPI_COMM_WORLD);
  if (rank == 0) {
    visitado[rank] = 1;
    printf("> MASTER: target => %d\n", target);
    for (int i = 1; i < size; i++) {
      if (conectado(rank, i)) {
        MPI_Send(visitado, size+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
        printf("mensaje: MASTER -> nodo %i\n", i);
      }
    }
  } /*
  else {
    // ----------------------
    MPI_Recv(mensaje, size+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("> Nodo %d:\n", rank);
    if (mensaje[size] != 0 && visitado[rank] == 0) {
      if (rank == target) {
        mensaje[size] = 1;
        printf("> Nodo %i: ENCONTRADO\n", rank);
      } else {
        for (int i = 0; i < size; i++) {
          if (i != rank) {
            if (conectado(rank, i)) {
              MPI_Send(mensaje, size+1, MPI_INT, i, 0, MPI_COMM_WORLD);
              MPI_Send(visitado, size, MPI_INT, i, 0, MPI_COMM_WORLD);
              printf("mensaje: Nodo %i -> Nodo %i \n", rank, i);
            }
          }
        }
      }
    }
    visitado[rank] = 1;
  }*/
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
    bfs(rank, size, target);
  MPI_Finalize();
  return 0;
}
