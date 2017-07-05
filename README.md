# Matrix Multiplication Collective #
### Soluzione ###
La soluzione proposta permette la moltiplicazione di due matrici A (m x n) e B (n x l) con m, n, l qualsiasi. Il numero dei processori, inoltre, non deve essere necessariamente divisore di m, n e l.
Le matrici sono state inserite all'interno di array monodimensionali per essere sicuri che occupassero sezioni di memoria contigue.
Il funzionamento dell'algoritmo è il seguente: le matrici A e B sono presenti in tutti i processori e ogni processore calcola l'array elemsPerProc:

```

int *elemsPerProc = malloc(sizeof(int) * size);
for(i = 0; i < size; i++)
  *(elemsPerProc + i) = (m % size > i) ? (m / size + 1) * n : (m / size) * n;

```

In questo array vengono specificati il numero di elementi della matrice A che ogni processore dovrà andare a moltiplicare. Nel caso in cui il numero di righe della matrie A non sia divisibile per il numero di processori, ai primi m%size processori viene assegnata una riga in più.
Per gestire il caso in cui il numero di righe della matrice A non fosse divisibile per il numero di processori è stata usata la MPI_Gatherv, che consente ai processori di restituire risultati di dimensione diversa.
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

Gli slave prima di invocare tale funzione devono calcolare il valore del proprio offset (la posizione nella matrice A dalla quale devono iniziare il calcolo). I dati computati dai vari processori, infine, vengono riuniti utilizzando la MPI_Gatherv.
### Risultati ###
Per effettuare il testing sono state utilizzate istanze m4.xlarge invece di m4.large in quanto è stato riscontrato il seguente comportamento anomalo nel corso dei test: eseguendo il programma su m4.large, il tempo di esecuzione aumentava in modo considerevole nel momento in cui si andava ad utilizzare più di un vCPU per istanza. Sulle m4.xlarge, invece, il problema si manifesta solo nel caso in cui vengano utilizzate più di 2 vCPU per istanza. Per effettuare i test, quindi, sono state utilizzate 8 istanze m4.xlarge sfruttando però solo 16 processori. Tale anomalia, inoltre, non è stata riscontrata utilizzando istanze t2.medium (2 vCPU) e in locale.
#### Strong scalability
m| n | l | istanze | processori | Tempo (secondi)
------------ | ------------ | ------------ | ------------ | ------------ |
2400 |2400 | 2400 | 1 | 1 | 160.59
2400 | 2400 | 2400 |	1 |	2 |	91.22
2400 | 2400 |	2400 | 2 | 3 | 83.57
2400 | 2400 | 2400 | 2 | 4 | 64.07
2400 | 2400 |	2400 | 3 | 5 | 51.43
2400 | 2400 |	2400 | 3 | 6 | 42.76
2400 | 2400	| 2400 | 4 | 7 | 36.74
2400 | 2400	| 2400 | 4 | 8 | 32.77
2400 | 2400	| 2400 | 5 | 9 | 29.18
2400 | 2400	| 2400 | 5 | 10 | 26.22
2400 | 2400	| 2400 | 6 | 11	| 23.81
2400 | 2400	| 2400 | 6 | 12	| 21.91
2400 | 2400	| 2400 | 7 | 13	| 20.24
2400 | 2400	| 2400 | 7 | 14	| 18.82
2400 | 2400	| 2400 | 8 | 15	| 17.53
2400 | 2400	| 2400 | 8 | 16	| 16.56

#### Weak scalability
m| n | l | istanze | processori | Tempo (secondi)
------------ | ------------ | ------------ | ------------ | ------------ |
150 | 2400 | 2400 |	1 |	1 | 10.45
300 |	2400 | 2400 |	1 |	2 |	11.35
450 |	2400 | 2400 |	2 |	3 |	15.98
600 |	2400 | 2400 |	2 |	4 |	16.14
750 |	2400 | 2400 |	3 |	5 |	16.27
900 |	2400 | 2400 |	3 |	6 |	16.13
1050 | 2400 |	2400 | 4 | 7 | 16.29
1200 | 2400 |	2400 | 4 | 8 | 16.37
1350 | 2400 | 2400 | 5 | 9 | 16.41
1500 | 2400 |	2400 | 5 | 10 |	16.42
1650 | 2400 |	2400 | 6 | 11 |	16.45
1800 | 2400 |	2400 | 6 | 12 |	16.43
1950 | 2400 |	2400 | 7 | 13 |	16.41
2100 | 2400 |	2400 | 7 | 14 |	16.52
2250 | 2400 |	2400 | 8 | 15 |	16.49
2400 | 2400 |	2400 | 8 | 16 |	16.51

### Esecuzione ###
Per l'esecuzione bisogna dare in input al programma i valori m, n, l:

```

mpirun -np num.processori --host MASTER,IP_SLAVE1,IP_SLAVE2 eseguibile m n l

```
