#include <omp.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double doTest11( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads, size_t threadPart )
{
	// each thread
	int rc;
	int iteration;
	int tid;
	MPI_Request requests[2];
	int other = (rank + 1) % 2;
	double start,duration;

#pragma omp parallel private(rc,iteration,tid,requests) num_threads(numThreads)
	{
		tid = omp_get_thread_num();

		if ( 0 == rank ) {
			rc = MPI_Send_init( (char*) sendBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, MPI_COMM_WORLD, &requests[1] );
			assert( rc == MPI_SUCCESS );
		} else {
			rc = MPI_Recv_init( (char*) recvBuf + tid * threadPart, threadPart, MPI_CHAR, other, tid, MPI_COMM_WORLD, &requests[0] );
			assert( rc == MPI_SUCCESS );
		}


#pragma omp master 
		start = MPI_Wtime();
		
		for ( iteration = 0; iteration < numIterations; iteration++ ) {

#pragma omp master 
			MPI_Barrier(MPI_COMM_WORLD);

#pragma omp barrier

			if ( 0 == rank ) {
				rc = MPI_Start(&requests[1]);
				assert( rc == MPI_SUCCESS );
				rc = MPI_Wait( &requests[1], MPI_STATUS_IGNORE);
				assert( rc == MPI_SUCCESS );
			} else {
				rc = MPI_Start(&requests[0]);
				assert( rc == MPI_SUCCESS );
				rc = MPI_Wait( &requests[0], MPI_STATUS_IGNORE);
				assert( rc == MPI_SUCCESS );
			}
		}

		if ( 0 == rank ) {
        	MPI_Request_free( &requests[1] );
		} else {
        	MPI_Request_free( &requests[0] );
		}
	}
	duration = MPI_Wtime() - start;

	return duration;
}
