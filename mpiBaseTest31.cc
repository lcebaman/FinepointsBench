#include <omp.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double doTest31( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads, size_t threadPart )
{
	int other = (rank + 1) % 2;
	double start;

#pragma omp parallel shared(rank,numIterations,sendBuf,recvBuf,numThreads,threadPart,other,start) num_threads(numThreads)
{
	int tid = omp_get_thread_num();
	int rc;
	int iteration;

#pragma omp master
	start = MPI_Wtime();

	for ( iteration = 0; iteration < numIterations; iteration++ ) {

#pragma omp barrier

		if ( 0 == rank ) {
			rc = MPI_Send( (char*) sendBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, MPI_COMM_WORLD );
			assert( rc == MPI_SUCCESS );
		} else {
			rc = MPI_Recv( (char*) recvBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			assert( rc == MPI_SUCCESS );
		}
	}
}

    return MPI_Wtime() - start;
}
