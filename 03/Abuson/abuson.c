#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?

#define IGNORE 0

/**
 * Inicializa la funcion random
 * @param id, identificador del nodo.
 */
void init_rand(int id) {
  time_t t;
	srand(time(&t)+id);
}

int nodosVivos(int n, int *protocolo){
  int vivos = 0;
  for (int i = 0; i < n; i++) {
    vivos += protocolo[i];
  }
  return vivos;
}

/**
 * Funcion para determinar cuantos y cuales nodos, estaran caidos.
 * @param n, tamaño de la red
 * @param lider_caido,
 * @param protocolo, arreglo que define el estado de la red
 * @return:
 */
void nodosCaidos(int n, int lider_caido, int *protocolo){
  protocolo[n] = 0;
  protocolo[n+1] = 0;
  for (int i = 0; i < n; i++) {
    protocolo[i] = 1;
  }
  protocolo[lider_caido] = 0;
  div_t r = div(n, 2);
  int nodosCaidos = (int) (rand()%r.quot)+1;
  while (nodosVivos(n, protocolo) != (n-nodosCaidos)) {
    protocolo[(int) rand()%n] = 0;
  }
}

/**
 * Funcion que define que nodo va a convocar a eleciones
 * @param n, tamaño de la red
 * @param protocolo, arreglo que define el estado de la red
 */
void eligeConvocante(int n, int *protocolo) {
  int convocante = 0;
  while (!protocolo[n+1]) {
    convocante = (int) rand()%n;
    protocolo[n+1] = protocolo[convocante];
  }
  protocolo[n+1] = convocante;
}

/**
 * Funcion principal
 * @param n, tamaño de la red.
 * @param id, identificador del nodo que invoca la funcion.
 * @param lider_caido
 */
void abuson(int n, int id, int lider_caido) {
  int protocolo[n+2];
  if (id == lider_caido) {
    nodosCaidos(n, lider_caido, protocolo);
    eligeConvocante(n, protocolo);
  }
  MPI_Bcast(protocolo, n+2, MPI_INT, lider_caido, MPI_COMM_WORLD);
  if (id == protocolo[n+1]) {
    // algoritmo elecciones
  }
}

int main(int argc, char* argv[]) {
  int rank, n, lider_caido;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &n);
    init_rand(rank);
    if (rank == 0) {
      lider_caido = (int) rand()%n;
    }
    MPI_Bcast(&lider_caido, 1, MPI_INT, 0, MPI_COMM_WORLD);
    abuson(n, rank, lider_caido);
  MPI_Finalize();
  return 0;
}
