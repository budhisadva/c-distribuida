#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?

#define PROBA 0.5

#define MASTER 0

#define TAG_IGNORE 0
#define TAG_SUCESS 1
#define TAG_FAIL 2

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
  if (u == v) return 0;
  if (u+1 == v) return 1;
  double p = (double)rand()/RAND_MAX;
  return p > PROBA;
}

/**
 * Garatiza que los valores iniciales del arreglo sean 0
 * @param arr, referencia del arreglo
 * @param tam, referencia del arreglo
 */
void setArreglo(int *arr, int tam){
   for (int i = 0; i < tam; i++) {
     arr[i] = 0;
   }
}

/*
 * Funcion axiliar que regresa el indice la primera posicion libre de la fila
 * @param f, referencia a la fila
 * @param n, tamñano de la fila (mismo de la red)
 * return el indice del primer lugar libre de la fila
 *        -1 en caso de que la fila este llena
 */
int determinaLugar(int *f, int n){
  int j = 1;
  while (f[j] != 0) {
    j++;
  }
  if (j < n) return j;
  return -1;
}

/**
 * bfs
 * @param rank, indice del nodo que manda a llamar
 * @param n, tamaño de la red
 * @param target, indice del nodo que buscamos
 */
void bfs(int rank, int n, int target) {
  int visitado[n+1];
  int fila[n];
  if (rank == 0) {
    setArreglo(visitado, n+1);
    setArreglo(fila, n);
  }
  MPI_Bcast(&visitado, n+1, MPI_INT, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&fila, n, MPI_INT, MASTER, MPI_COMM_WORLD);
  // MASTER
  if (rank == 0) {
    fila[0] = 1;
    visitado[0] = 1;
    printf("> MASTER => Buscar: Nodo %i\n", target);
    int j = 1;
    for (int i = 1; i < n; i++) {
      if (conectado(rank, i)) {
        visitado[i] = 1;
        fila[j] = i;
        j++;
      }
    }
    for (int i = 1; i < n; i++) {
      if (fila[i] != 0) {
        printf("> MASTER => enviado a <Nodo %i>\n", fila[i]);
        MPI_Send(&visitado, n+1, MPI_INT, fila[i], TAG_IGNORE, MPI_COMM_WORLD);
        MPI_Send(&fila, n, MPI_INT, fila[i], TAG_IGNORE, MPI_COMM_WORLD);
      }
    }
    // Espera de una respuesta exitosa
    MPI_Recv(&visitado, n+1, MPI_INT, MPI_ANY_SOURCE, TAG_SUCESS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("%s\n", "MASTER => Busqueda terminada.");
    for (int i = 1; i < n; i++) {
      if (!visitado[i]) {
        MPI_Send(&visitado, n+1, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
        MPI_Send(&fila, n, MPI_INT, i, TAG_IGNORE, MPI_COMM_WORLD);
      }
    }
  }
  // RESTO DE NODOS
  else {
    MPI_Status status;
    MPI_Recv(&visitado, n+1, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&fila, n, MPI_INT, MPI_ANY_SOURCE, TAG_IGNORE, MPI_COMM_WORLD, &status);
    // El target no ha sido encontrado
    if (!visitado[n]) {
      // el nodo actual es el objetivo
      if (rank == target) {
        visitado[n] = 1;
        printf("> Nodo %i => SOY EL OBJETIVO\n", rank);
        MPI_Send(&visitado, n+1, MPI_INT, MASTER, TAG_SUCESS, MPI_COMM_WORLD);
        printf("> Nodo %i => enviado a MASTER\n", rank);
      }
      else {
        // determinar indice del primer lugar libre de la fila
        int l = determinaLugar(fila, n);
        if (l != -1) {
          int k = l;
          // Agregar a la fila nodos no visitados y conectado
          for (int i = 1; i < n; i++) {
            if (!visitado[i] && conectado(rank, i)) {
              visitado[i] = 1;
              fila[k] = i;
              k++;
            }
          }
          while (fila[l] != 0 && l < n) {
            printf("> Nodo %i => enviado mensaje a <Nodo %i>\n", rank, fila[l]);
            MPI_Send(&visitado, n+1, MPI_INT, fila[l], TAG_IGNORE, MPI_COMM_WORLD);
            MPI_Send(&fila, n, MPI_INT, fila[l], TAG_IGNORE, MPI_COMM_WORLD);
            l++;
          }
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
    bfs(rank, n, target);
  MPI_Finalize();
  return 0;
}
