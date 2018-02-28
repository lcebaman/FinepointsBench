#include <omp.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double doTest32( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads, size_t threadPart )
{
	int other = (rank + 1) % 2; 
	double start = MPI_Wtime();

#pragma omp parallel shared(numIterations,sendBuf,recvBuf,threadPart,other) num_threads(numThreads)// default(none)
{
	int rc;
	int tid = omp_get_thread_num();
	int iteration;
	MPI_Request requests[2];

	for ( iteration = 0; iteration < numIterations; iteration++ ) {

#pragma omp barrier

		rc = MPI_Isend( (char*) sendBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, 
							MPI_COMM_WORLD, &requests[ 0 ] );
		assert( rc == MPI_SUCCESS );

		rc = MPI_Irecv( (char*) recvBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, 
							MPI_COMM_WORLD, &requests[ 1 ] );
		assert( rc == MPI_SUCCESS );

		rc = MPI_Waitall( 2, requests , MPI_STATUS_IGNORE );
		assert( rc == MPI_SUCCESS );
	}
}
	
	return MPI_Wtime() - start;
}
