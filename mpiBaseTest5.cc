#include <omp.h>
#include <time.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double doTest5( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads, size_t threadPart, double compTime, double noise )
{
	// each thread
	double start,duration;

	start = MPI_Wtime();

    long sleep = compTime * 1000000000;
    long sleepPlus = (compTime + ( compTime * noise)) * 1000000000 ;

#pragma omp parallel shared(rank,numIterations,sendBuf,recvBuf,sleep,sleepPlus) num_threads(numThreads)
{
	int other = (rank + 1) % 2;
	int rc;
	int tid = omp_get_thread_num();
	int iteration;

	struct timespec req,rem;
	req.tv_sec = 0;

    if ( numThreads > 1 && tid == numThreads - 1 ) {
		req.tv_nsec = sleepPlus;
    } else {
		req.tv_nsec = sleep;
    }
	size_t bufSize = numThreads * threadPart;
	for ( iteration = 0; iteration < numIterations; iteration++ ) {

		if ( 0 == rank ) {

#pragma omp barrier	
            rc = clock_nanosleep(CLOCK_REALTIME,0,&req, &rem);
            if ( 0  !=  rc ) {
                printf("rc=%s rem %li\n",strerror(rc),rem.tv_nsec);
            }
#pragma omp barrier	

#pragma omp master
{
			rc = MPI_Send( (char*) sendBuf, bufSize, MPI_CHAR, other, 0xdead, MPI_COMM_WORLD );
			assert( rc == MPI_SUCCESS );
}
		} else {
#pragma omp barrier	
#pragma omp master
{
			rc = MPI_Recv( (char*) recvBuf, bufSize, MPI_CHAR, other, 0xdead, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			assert( rc == MPI_SUCCESS );
}
		}
	}
}

	duration = MPI_Wtime() - start;
	if ( numThreads > 1 ) {
		duration -=  sleepPlus / 1000000000.0 * numIterations;
	} else {
		duration -=  sleep / 1000000000.0 * numIterations;
	}

	return duration;
}
