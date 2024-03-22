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
 * @param s,tamaño de la red
 * return 1 si p > 0.5 o si v es el siguiente inmediato de u
 *        0 en otro caso
 */
int conectado(int u, int v, int s){
  if (v == s) return 1;
  double p = (double)rand()/RAND_MAX;
  return p > PROBA;
}

/**
 * dfs
 * @param rank, indice del nodo que manda a llamar
 * @param size, tamaño de la red
 * @param target, indice del nodo que buscamos
 */
void dfs(int rank, int size, int target) {
  int visitado[size+1];
  MPI_Bcast(&visitado, size, MPI_INT, MASTER, MPI_COMM_WORLD);
  if (rank == 0) {
    /* code */
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
