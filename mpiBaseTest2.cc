#include <omp.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpiPart.h>


double doTest2( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads, size_t threadPart )
{
	int other = (rank + 1) % 2;
	double start;
	int TAG = 0xdead;
    MPI_Request sendReq,recvReq;
	int rc;

    if ( rank == 0 ) {
		rc = MPI_Partitioned_Send_create( sendBuf, threadPart * numThreads, MPI_CHAR, numThreads, other, TAG,
                    MPI_COMM_WORLD, &sendReq );
	    assert( rc == MPI_SUCCESS );

   	    rc = MPI_Partitioned_Recv_create( recvBuf, threadPart * numThreads, MPI_CHAR, other, TAG,
                   	MPI_COMM_WORLD, &recvReq );
	    assert( rc == MPI_SUCCESS );
	} else {
   	    rc = MPI_Partitioned_Recv_create( recvBuf, threadPart * numThreads, MPI_CHAR, other, TAG,
                    MPI_COMM_WORLD, &recvReq );
	    assert( rc == MPI_SUCCESS );

   	    rc = MPI_Partitioned_Send_create( sendBuf, threadPart * numThreads, MPI_CHAR, numThreads, other, TAG,
                   	MPI_COMM_WORLD, &sendReq );
	    assert( rc == MPI_SUCCESS );
    }

	start = MPI_Wtime();

#pragma omp parallel shared(rank,numIterations,sendBuf,recvBuf,numThreads,threadPart,sendReq,recvReq) num_threads(numThreads)
	{
	    int rc;
	    int iteration;
		int tid = omp_get_thread_num();

#pragma omp master 
        assert( numThreads == omp_get_num_threads() );

		for ( iteration = 0; iteration < numIterations; iteration++ ) {

#pragma omp master 
			{
				rc = MPI_Start_part(&sendReq);
				assert( rc == MPI_SUCCESS );
			}

#pragma omp barrier
			rc = MPI_Partitioned_Add_to_buffer( &sendReq, sendBuf + (threadPart * tid), threadPart, MPI_CHAR );
            assert( rc  == MPI_SUCCESS );
#pragma omp barrier

#pragma omp master 
			{
				rc = MPI_Wait_part(&recvReq, MPI_STATUS_IGNORE );
				assert( rc == MPI_SUCCESS );
			}
		}
	}

	return MPI_Wtime() - start;
}
