#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define WORKTAG 1
#define DIETAG 2
#define CHUNKTAG 3
#define ARRAYSIZE 160000



float data[ARRAYSIZE];

  /* Local functions */

static void master(void);
static void slave(void);
static void get_next_work_item(int,int*);
static float do_work(int);



int main(int argc, char **argv)
{

	int myrank;

  /* Inizializzazione MPI */
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if (myrank == 0)
		master();
	else
		slave();

  /* Finalizzo MPI */
	MPI_Finalize();

	return 0;
}


static void master(void)
{
	int ntasks, work;
	float result;
	int chunksize, dest, n;
	
	double starttime, endtime;
	MPI_Status status;

  /* Acquisizione del numero di processi */
	MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

  /* Inizializzo array, inserisco dati casuali */
	int rank;
	for(rank = 0; rank < ARRAYSIZE; ++rank) 
		data[rank] =  rank * 1.0;

	printf("Dimensione Array: %d \nInserire numero di parti in cui si vuole suddividere il lavoro: \n", ARRAYSIZE);
	scanf("%d",&n);	
	if(!(ARRAYSIZE % n == 0)){
		printf("Errore, il coefficiente inserito non divide l'array in parti intere. \n");
		exit(1);
        }

  /* Misuro il tempo di esecuzione del lavoro */	
	starttime = MPI_Wtime();
	
	chunksize = (ARRAYSIZE / n);

  /* Ottenimento primo lavoro */
	work = 0;
  	get_next_work_item(0,&work);	
	
  /* Assegnamento del lavoro a ciascuno slave (rank da 1 a ntasks-1). Ogni schiavo riceve la dimensione del lavoro e la porzione di array su cui dovrà operare. */
	for (dest=1; dest<ntasks; ++dest) 
	{
		MPI_Send(&chunksize, 1, MPI_INT, dest, CHUNKTAG, MPI_COMM_WORLD);		
		MPI_Send(&data[work], chunksize, MPI_FLOAT, dest, WORKTAG, MPI_COMM_WORLD);

  		get_next_work_item(chunksize,&work);
	}
	
	float avrg = 0;

	while (work != -1) {


		MPI_Recv(&result, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  		avrg += result;

    /* Invio lavoro successivo: */
		MPI_Send(&chunksize, 1, MPI_INT, status.MPI_SOURCE, CHUNKTAG, MPI_COMM_WORLD);		
		MPI_Send(&data[work], chunksize, MPI_FLOAT, status.MPI_SOURCE, WORKTAG, MPI_COMM_WORLD);
		
		get_next_work_item(chunksize,&work);
	}

    /* Finiti i lavori il master riceve tutti gli ultimi risultati in lavorazione */

	for (rank = 1; rank < ntasks; ++rank)
	{
		MPI_Recv(&result, 1, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		avrg += result;
	}

    /* Il master infine ordina a ciascuno slave di terminare l'esecuzione tramite l'invio del tag DIETAG. */
	for (rank = 1; rank < ntasks; ++rank)
		MPI_Send(0, 0, MPI_INT, rank, DIETAG, MPI_COMM_WORLD);
	
	avrg /= n;

	printf("La media degli elementi nell'array è: %f\n", avrg);

	endtime = MPI_Wtime();
	printf("Tempo totale sul master: %f sec\n",(endtime - starttime));
}


static void slave(void)
{
	int chunk;
	float result;
	
	MPI_Status status;

	double starttimeS, endtimeS;
	starttimeS = MPI_Wtime();

	while (1) 
	{

		MPI_Recv(&chunk, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  /* Controlla il tipo di messaggio ricevuto tramite il tag, se il tag del messaggio 
	ricevuto coincide con il tag che decreta il termine del lavoro esce dal programma. */	
		if (status.MPI_TAG == DIETAG)
		{
			endtimeS = MPI_Wtime();
			printf("Tempo di esecuzione sul worker: %f sec\n",(endtimeS - starttimeS));
			return;
		}

		MPI_Recv(&data[0], chunk, MPI_FLOAT, 0, WORKTAG, MPI_COMM_WORLD, &status);

  /* Chiamata alla funzione che esegue il lavoro */
		result = do_work(chunk);
		
  /* Invio del risultato parziale al master (rank = 0) */
		MPI_Send(&result, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
	
	}

	
}

/* La funzione get_next_work_item riceve come argomenti la dimensione del lavoro e il riferimento alla variabile che tiene conto dell'ultimo lavoro assegnato dal master. count viene aggiornata dalla funzione e, modificando il puntatore si modifica anche la variabile puntata da esso che appartiene al main: cosi viene ottenuto il nuovo lavoro. Nel caso i lavori fossero esauriti count assume un valore convenzionale. */
static void get_next_work_item(int chunk, int *count)
{

	if(chunk == 0)
	{
		*count = 0;		
		return;
	}
	else{
		*count += chunk;
    /* Controllo di avere ancora lavori disponibili */
		if(*count > (ARRAYSIZE-1))
			*count = -1;
		return;

	}
}

  /* Il programma calcola la media degli elementi nell'array data. */ 
static float do_work(int chunk)
{
	int i;
	float avg = 0.0;

	for(i=0; i < ARRAYSIZE; i++)	
		avg += data[i];
	avg /= chunk;
	return avg;
}
