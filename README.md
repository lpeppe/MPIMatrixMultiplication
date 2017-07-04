# Matrix Multiplication Collective #
### Soluzione ###
La soluzione proposta permette la moltiplicazione di due matrici A (m x n) e B (n x l) con m, n, l qualsiasi. Il numero dei processori, inoltre, non deve essere necessariamente divisore di m, n e l.
Il funzionamento dell'algoritmo è il seguente: le matrici A e B sono presenti in tutti i processori e ogni processore calcola l'array elemsPerProc:

```

int *elemsPerProc = malloc(sizeof(int) * size);
for(i = 0; i < size; i++)
  *(elemsPerProc + i) = (m % size > i) ? (m / size + 1) * n : (m / size) * n;

```

In questo array vengono specificati il numero di elementi della matrice A che ogni processore dovrà andare a moltiplicare.  
Il master a questo punto procede a calcolare gli array revcounts e displs che successivamente utilizzerà nella MPI_Gatherv. La funzione computeSubResult viene utilizzata per computare la porzione di matrice C che spetta ad ogni processore:

```
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

```

Gli slave prima di invocare tale funzione devono calcolare il valore del proprio offset (la posizione nella matrice A dalla quale devono iniziare il calcolo). I dati computati dai vari processori, infine, vengono riuniti utilizzando la MPI_Gatherv
