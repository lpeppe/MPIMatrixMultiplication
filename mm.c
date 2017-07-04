#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#define MAXINT 20

void printMatrix(long *matrix, int m, int l) {
  FILE *fp;
  fp = fopen("text.txt", "w+");
  int i;
  for(i = 0; i < m * l; i++) {
    if(i % l == 0)
      fprintf(fp, "\n");
    fprintf(fp, "%ld ", *(matrix + i));
  }
  fclose(fp);
}

void computeSubResult(int *mat1, int *mat2, long *result, int elemsPerProc, int offset, int m, int n, int l) {
  int i = 0, j = 0, k = 0, cont = 0;
  long sum = 0;
  for(k = 0; k < elemsPerProc / n; k++) {
    for(i = 0; i < l; i++) {
      for(j = 0; j < n; j++)
        sum += * (mat1 + j + (k * n) + offset) * *(mat2 + i + (l * j));
      *(result + cont++) = sum;
      sum = 0;
    }
  }
}

int main(int argc, char *argv[]) {
  //mat1 is a MxN matrix, mat2 a NxL matrix
  if(argc != 4) {
    printf("Arguments error!\n");
    return 1;
  }
  int m = atoi(argv[1]), n = atoi(argv[2]), l = atoi(argv[3]);
  int rank, size;
  char hostname[50];
  int length, i;
  gethostname(hostname, length);
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int *mat1 = malloc(sizeof(int) * m * n);
  for(i = 0; i < m * n; i++)
    *(mat1 + i) = rand() % MAXINT;
  /*printf("Prima matrice:\n");
  for(i = 0; i < m * n; i++) {
     if(i%n == 0)
        printf("\n");
     printf("%d ", *(mat1 + i));
  }*/
  int *mat2 = malloc(sizeof(int) * n * l);
  for(i = 0; i < n * l; i++)
    *(mat2 + i) = rand() % MAXINT;
  /*printf("\n\nSeconda matrice:\n");
  for(i = 0; i < n * l; i++) {
     if(i%l == 0)
        printf("\n");
     printf("%d ", *(mat2 + i));
  }*/
  int *elemsPerProc = malloc(sizeof(int) * size);
  for(i = 0; i < size; i++)
    *(elemsPerProc + i) = (m % size > i) ? (m / size + 1) * n : (m / size) * n;
  if(rank == 0) {
    double startTime, endTime;
    startTime = MPI_Wtime();
    int *revcounts = malloc(sizeof(int) * size);
    for(i = 0; i < size; i++)
      *(revcounts + i) = *(elemsPerProc + i) / n * l;
    int *displs = malloc(sizeof(int) * size);
    *(displs) = 0;
    for(i = 1; i < size; i++)
      *(displs + i) = *(displs + i - 1) + *(revcounts + i - 1);
    long *result = malloc(sizeof(long) * *(revcounts + rank));
    computeSubResult(mat1, mat2, result, *(elemsPerProc + rank), 0, m, n, l);
    long *recv_data = malloc(sizeof(long) * m * l);
    MPI_Gatherv(result, *(revcounts + rank), MPI_LONG, recv_data,
      revcounts, displs, MPI_LONG, 0, MPI_COMM_WORLD);
    endTime = MPI_Wtime();
    printf("\n\nTotal time: %f\n\n", endTime - startTime);
    //printMatrix(recv_data, m, l);
    free(recv_data);
    free(revcounts);
    free(displs);
    free(result);
  }
  else {
    int offset = 0;
    for(i = 0; i < rank; i++)
      offset += *(elemsPerProc + i);
    long *result = malloc(sizeof(long) * (*(elemsPerProc + rank) / n * l));
    computeSubResult(mat1, mat2, result, *(elemsPerProc + rank), offset, m, n, l);
    MPI_Gatherv(result, *(elemsPerProc + rank) / n * l, MPI_LONG, NULL,
      NULL, NULL, MPI_LONG, 0, MPI_COMM_WORLD);
    free(result);
  }
  free(mat1);
  free(mat2);
  free(elemsPerProc);
  MPI_Finalize();
  return 0;
}
