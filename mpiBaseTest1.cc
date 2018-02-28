#include <omp.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double doTest1( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads, size_t threadPart )
{
	int other = (rank + 1) % 2;
	double start;
	MPI_Request requests[2*numThreads];

#pragma omp parallel shared(rank,numIterations,sendBuf,recvBuf,numThreads,threadPart,other,start,requests) num_threads(numThreads)

	{
		int rc;
		int iteration;
		int tid = omp_get_thread_num();

		rc = MPI_Recv_init( (char*) recvBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, MPI_COMM_WORLD, &requests[tid*2 + 0] );
		assert( rc == MPI_SUCCESS );

		rc = MPI_Send_init( (char*) sendBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, MPI_COMM_WORLD, &requests[tid*2 + 1] );
		assert( rc == MPI_SUCCESS );

#pragma omp master 
		start = MPI_Wtime();
		
		for ( iteration = 0; iteration < numIterations; iteration++ ) {

#pragma omp master
			rc = MPI_Barrier(MPI_COMM_WORLD);
			assert( rc == MPI_SUCCESS );
#pragma omp barrier

			rc = MPI_Start(&requests[tid*2 + 0]);
			assert( rc == MPI_SUCCESS );

			rc = MPI_Start(&requests[tid*2 + 1]);
			assert( rc == MPI_SUCCESS );

			rc = MPI_Waitall( 2, &requests[ tid*2 ], MPI_STATUS_IGNORE );
			assert( rc == MPI_SUCCESS );
		}

		MPI_Request_free( &requests[tid*2 + 0] );
		MPI_Request_free( &requests[tid*2 + 1] );

	}
	return  MPI_Wtime() - start;
}
