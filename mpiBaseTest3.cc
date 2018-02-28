#include <omp.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double doTest3( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads, size_t threadPart )
{
	int other = (rank + 1) % 2;
	double start;
	MPI_Request requests[2*numThreads];

#pragma omp parallel shared(requests,rank,numIterations,sendBuf,recvBuf,numThreads,threadPart,other,start) num_threads(numThreads) //default(none)
{
	int rc;
	int tid = omp_get_thread_num();
	int iteration;

#pragma omp master
	{
		assert( numThreads == omp_get_num_threads() );	
		start = MPI_Wtime();
	}

	for ( iteration = 0; iteration < numIterations; iteration++ ) {

		//printf("rank=%d thread=%d iteration=%d myPart=%lu\n",rank,tid,iteration,threadPart);
#pragma omp barrier

		rc = MPI_Isend( (char*) sendBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, 
							MPI_COMM_WORLD, &requests[ tid * 2 ] );
		assert( rc == MPI_SUCCESS );

		rc = MPI_Irecv( (char*) recvBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, 
							MPI_COMM_WORLD, &requests[ tid * 2 + 1] );
		assert( rc == MPI_SUCCESS );

		rc = MPI_Waitall( 2, &requests[tid*2] , MPI_STATUS_IGNORE );
		assert( rc == MPI_SUCCESS );
	}
}
	
	return MPI_Wtime() - start;
}
