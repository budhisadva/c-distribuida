#include <mpi.h>    //Libreria OpenMPI
#include <stdio.h>  //Entrada y salida
#include <stdlib.h> //Libreria del random
#include <time.h>   //Funciones de tiempo
#include <string.h> // ?
#include <unistd.h>

#define ELECCION 0
#define RESPUESTA 1
#define COORDINADOR 2

/**
 * Inicializa la funcion random
 * @param id, identificador del nodo.
 */
void init_rand(int id) {
  time_t t;
	srand(time(&t)+id);
}

// solo usar dentro del main
void imprimeEstadoNodos(int n, int *protocolo){
  for (int i = 0; i < n; i++) {
    if (protocolo[i]) {
      printf(">Nodo %i: ON\n", i);
    } else {
      printf(">Nodo %i: OFF\n", i);
    }
  }
}

/**
 * Devuelve la cantidad de nodos que no han caido
 * @param n, tamaño de la red
 * @param protocolo, arreglo que define el estado de la red
 */
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
 * Funcion que define que nodo va a convocar a elecciones
 * @param n, tamaño de la red
 * @param protocolo, arreglo que define el estado de la red
 * @return; el convocante esta en protocolo[n+1]
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
 * Devuelve el indice del nodo mas alto y que no haya caido
 * @param n, tamaño de la red
 * @param protocolo, arreglo que define el estado de la red
 * @return
 */
int noFallidoMasAlto(int n, int *protocolo){
  int i = 0, nodo = 0;
  while (i < n) {
    if (protocolo[i]) {
      nodo = i;
    }
    i++;
  }
  return nodo;
}

/**
 * Determina quien sera el nuevo coordinador
 * @param n, tamaño de la red
 * @param id, identificador del nodo
 * @param protocolo, arreglo que define el estado de la red
 * @return
 */
void elecciones(int n, int id, int *protocolo) {
  MPI_Status status;
  // 1. Si un proceso sabe que tiene el id (no fallido) más alto:
  if (id == noFallidoMasAlto(n, protocolo)) {
    printf(">Nodo %i [CONVOCANTE]: [CONVOCANTE] -> [COORDINADOR]\n", id);
    protocolo[n] = id;
    for (int i = 0; i < id; i++) {
      printf(">Nodo %i [COORDINADOR]: => Nodo %i, tipo COORDINADOR\n", id, i);
      MPI_Send(&protocolo, n+2, MPI_INT, i, COORDINADOR, MPI_COMM_WORLD);
    }
  } else {
    // 2. Manda un mensaje elección a todos los de id más alto y espera un mensaje respuesta
    for (int i = id+1; i < n; i++) {
      printf(">Nodo %i [CONVOCANTE]: => Nodo %i, tipo ELECCION\n", id, i);
      MPI_Send(&protocolo, n+2, MPI_INT, i, ELECCION, MPI_COMM_WORLD);
    }
    printf(">Nodo %i [CONVOCANTE]: En espera de RESPUESTA\n", id);
    // Simular timeout
    MPI_Recv(&protocolo, n+2, MPI_INT, MPI_ANY_SOURCE, RESPUESTA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf(">Nodo %i [CONVOCANTE]: En espera de COORDINADOR\n", id);
    MPI_Recv(&protocolo, n+2, MPI_INT, MPI_ANY_SOURCE, COORDINADOR, MPI_COMM_WORLD, &status);
    printf(">Nodo %i [CONVOCANTE]: Nuevo coordinador recibido\n", id);
    protocolo[n] = (int) status.MPI_SOURCE;
    //
  }
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
  MPI_Bcast(&protocolo, n+2, MPI_INT, lider_caido, MPI_COMM_WORLD);
  // En caso de que sea el nodo convocante
  if (id == protocolo[n+1]) {
    printf(">Nodo %i [CONVOCANTE]: Lider caido\n", id);
    imprimeEstadoNodos(n, protocolo);
    elecciones(n, id, protocolo);
  } else {
    if (protocolo[id]) {
      MPI_Status status;
      MPI_Recv(&protocolo, n+2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (status.MPI_TAG == 0) {
        printf(">Nodo %i: <= Nodo %d, de tipo ELECCION\n", id, status.MPI_SOURCE);
        MPI_Send(&protocolo, n+2, MPI_INT, status.MPI_SOURCE, RESPUESTA, MPI_COMM_WORLD);
        printf(">Nodo %i: => Nodo %d, de tipo RESPUESTA\n", id, status.MPI_SOURCE);
        elecciones(n, id, protocolo);
      } else if (status.MPI_TAG == 2) {
        printf(">Nodo %i: <= Nodo %d, de tipo COORDINADOR\n", id, status.MPI_SOURCE);
        protocolo[n] = (int) status.MPI_SOURCE;
      }
    }
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
