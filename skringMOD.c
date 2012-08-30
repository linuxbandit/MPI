/*******************************************
 * Questo programma e' stato sviluppato da:
 * Open Systems Laboratory
 * http://www.lam-mpi.org/tutorials/
 * Indiana University
 *
 * Gli n processi comunicano su un anello:
 * il processo 0 iniva al processo 1 e riceve da n-1;
 * 1 riceve da 0 e invia a 2;
 * ... 
 * n-1 riceve da n-2 e invia a 0;
 * 
 * Il codice seguente fornisce la struttura generale del 
 * programma e' necessario completarlo con le opportune primitive
 * MPI per lo scambio di messaggi (MPI_Send, MPI_Recv)
 */

#include <stdio.h>
#include <mpi.h>


int main(int argc, char *argv[])
{
  MPI_Status status;
  int num, rank, size, tag, next, from;

  /* Start up MPI */
  MPI_Init(&argc, &argv); 


  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Determina la dimensione del gruppo di processori */
  MPI_Comm_size(MPI_COMM_WORLD, &size);



  tag = 201;
  next = (rank + 1) % size;
  from = (rank + size - 1) % size;
   
  /* Se siamo il processo "console", ossia quello 
     con rank nullo, [vuol dire che sta partendo un nuovo giro!]
     chiediamo all'utente di inserire un numero intero, 
     per specificare quante volte vogliamo "girare" nell'anello 
  */

  if (rank == 0) 
  {
    printf("Enter the number of times around the ring: \n");
    scanf("%d", &num);

    printf("Process %d sending %d to %d\n", rank, num, next);

    /*Invia il numero al prossimo processo nel ring: 
	- MPI_Send(&buf, count, MPI_Datatype, dest, tag, MPI_COMM_WORLD);
	- MPI_Recv(&buf, count, MPI_Datatype, source, tag, MPI_COMM_WORLD, &status);
	La sintassi di MPI_Send() e MPI_Recv()è la seguente: 
	buf punta al primo elemnto del dato che si vuole inviare, oppure, qualora il dato debba essere ricevuto è la destinazione. 
	count è il numero di elementi di tipo MPI_Datatype da trasferire alla destinazione dest dalla sorgente source. 
	status contiene informazioni relative al messaggio.

    */ 
    MPI_Send(&num, 1, MPI_INT, next, tag, MPI_COMM_WORLD); 
  }


  do {
    /* Riceve il numero,
	se il processo console non ha iniziato ad inviare, gli altri si fermano in attesa del dato. */

    MPI_Recv(&num, 1, MPI_INT, from, tag, MPI_COMM_WORLD, &status);
    
    printf("Process %d received %d\n", rank, num);

    if (rank == 0) 
    {
      --num;
      printf("Process 0 decremented num\n");
    }

    printf("Process %d sending %d to %d\n", rank, num, next);
    
    /* Invia il numero al prossimo processo del ring */
    MPI_Send(&num, 1, MPI_INT, next, tag, MPI_COMM_WORLD);
  } while (num > 0);

  printf("Process %d exiting\n", rank);

 /* L'ultimo processo effettua un invio ulteriore al processo 0, che si pone in attesa di questo prima di poter uscire */

  if (rank == 0)
      MPI_Recv(&num, 1, MPI_INT, from, tag, MPI_COMM_WORLD, &status);
 
  /* Quit */
      MPI_Finalize();
  
  return 0;
}

