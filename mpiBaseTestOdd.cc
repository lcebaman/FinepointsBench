#include <omp.h>
#include <time.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpiPart.h>
#include <cstdlib>
#include <random>
#include <ctime>

//#define VERIFY 1
double doTestOdd( int rank, int numIterations, char* sendBuf, char* recvBuf, int numThreads,
		  size_t bufSize, double compTime, double noise )
{
  
  MPI_Request sendReq,recvReq;
  int other = (rank + 1) % 2;
  double start;
  int TAG = 0xdead;
  int rc;

  size_t threadPart = bufSize/numThreads;
  
#ifdef VERIFY
  for ( int i = 0; i < (threadPart * numThreads) / 8; i++ ) {
    ((uint64_t*)sendBuf)[i] = i;
  }

  bzero( recvBuf, threadPart * numThreads );
#endif
  

  
  if ( rank == 0 ) {
    rc = MPI_Partitioned_Send_create( sendBuf, bufSize, MPI_CHAR,
				      numThreads, other, TAG,
                                      MPI_COMM_WORLD, &sendReq );
    assert( rc == MPI_SUCCESS );
    
  } else {
    rc = MPI_Partitioned_Recv_create( recvBuf, bufSize, MPI_CHAR, other, TAG,
                                      MPI_COMM_WORLD, &recvReq );
    assert( rc == MPI_SUCCESS );
  }

  start = MPI_Wtime();

  srand(time(NULL));

  long sleep = compTime * 1000000000;
  long sleepPlus = (compTime + ( compTime * noise)) * 1000000000 ;

#pragma omp parallel shared(rank,numIterations,sendBuf,recvBuf,threadPart,sendReq,recvReq,sleep,sleepPlus,bufSize) num_threads(numThreads)
  {
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

    for ( iteration = 0; iteration < numIterations; iteration++ ) {


      if ( 0 == rank ) {

#pragma omp master
        {
          rc = MPI_Start_part(&sendReq);
          assert( rc == MPI_SUCCESS );
        }

#pragma omp barrier

        rc = clock_nanosleep(CLOCK_REALTIME,0,&req, &rem);
        if ( 0  !=  rc ) {
          printf("rc=%s rem %li\n",strerror(rc),rem.tv_nsec);
        }
        rc = MPI_Partitioned_Add_to_buffer_a( &sendReq, sendBuf + (threadPart * tid),
					    threadPart, MPI_CHAR );
        assert( rc  == MPI_SUCCESS );

#pragma omp barrier
      }

      if ( 1 == rank ) {
#pragma omp master
        {
          rc = MPI_Wait_part(&recvReq, MPI_STATUS_IGNORE );
          assert( rc == MPI_SUCCESS );
#ifdef VERIFY
          for ( int i = 0; i < (threadPart * numThreads) / 8; i++ ) {
            assert( ((uint64_t*)recvBuf)[i] == i );
          }
          bzero( recvBuf, threadPart * numThreads );
#endif
        }
      }
    }
  }
  if ( 0 == rank ) {
    MPI_Partitioned_free( &sendReq );
  } else {
    MPI_Partitioned_free( &recvReq );
  }

  double duration = MPI_Wtime() - start;
  if ( numThreads > 1 ) {
    duration -=  sleepPlus / 1000000000.0 * numIterations;
  } else {
    duration -=  sleep / 1000000000.0 * numIterations;
  }
  return duration;
}
