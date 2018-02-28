#include <omp.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string>

double doTest1( int rank, int, char*, char*, int, size_t );
double doTest11( int rank, int, char*, char*, int, size_t );
double doTest2( int rank, int, char*, char*, int, size_t );
double doTest21( int rank, int, char*, char*, int, size_t, double, double );
double doTestOdd( int rank, int, char*, char*, int, size_t, double, double );
double doTest3( int rank, int, char*, char*, int, size_t );
double doTest31( int rank, int, char*, char*, int, size_t );
double doTest31t( int rank, int, char*, char*, int, size_t );
double doTest32( int rank, int, char*, char*, int, size_t );
double doTest5( int rank, int, char*, char*, int, size_t, double, double );

struct Args {
  int threadModel;
  int test;
  int numIterations;
  int numThreads;
  int minNumThreads;
  int maxNumThreads;
  size_t maxBuffSize;
  size_t minBuffSize;
  double compTime;
  double noise;
};

int parseArgs( int argc, char* argv[], Args* args );
std::string getThreadModelStr(int model);

int main( int argc, char* argv[] )
{
  int provided;
  int rank, size;
  int rc;

  Args args;
  args.threadModel = MPI_THREAD_MULTIPLE;
  args.test = 1;
  args.numThreads = 0;
  args.numIterations = 1;
  args.minNumThreads = 2;
  args.maxNumThreads = 1;
  args.minBuffSize = 12421;
  args.maxBuffSize = 12421;
  args.compTime = 0.050;
  args.noise = 0.002;

  rc = parseArgs( argc, argv, &args );
  if ( -1 == rc ) {
    return -1;
  }

  if ( args.numThreads != 0 ) {
    args.minNumThreads = args.maxNumThreads = args.numThreads;
  }

  rc = MPI_Init_thread( &argc, & argv, args.threadModel, &provided );
  assert( rc  == MPI_SUCCESS );

  assert( provided == args.threadModel );

  rc = MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  assert( rc  == MPI_SUCCESS );

  if ( 0 == rank ) {
    printf("# OMP_NUM_THREADS=%s\n",getenv("OMP_NUM_THREADS")?getenv("OMP_NUM_THREADS"):" not set");
    printf("# MPI thread model %s\n",getThreadModelStr(args.threadModel).c_str() );
    printf("#  iterations=%d minThreads=%d maxThreads=%d minBuf=%lu maxBuf=%lu\n",
	   args.numIterations, args.minNumThreads,args.maxNumThreads,
	   args.minBuffSize,args.maxBuffSize);
    
  }

  
  rc = MPI_Comm_size( MPI_COMM_WORLD, &size );
  assert( rc  == MPI_SUCCESS );

  assert( size == 2 );

  char* sendBuf = (char*) malloc( args.maxBuffSize );
  char* recvBuf = (char*) malloc( args.maxBuffSize );

  size_t bufSize = args.maxBuffSize;
  int numThreads = 7;

  doTestOdd( rank, args.numIterations, sendBuf, recvBuf, numThreads,
	     bufSize, args.compTime, args.noise  );
  

  rc = MPI_Finalize();
  assert( rc  == MPI_SUCCESS );

  printf("# %d: rank=%d exiting\n",__LINE__,rank);

  return 0;
}
int parseArgs( int argc, char* argv[], Args* args )
{
#define OPTIONAL 2
  static struct option long_options[] = {
    {"i", OPTIONAL, NULL, 0 },
    {"minT", OPTIONAL, NULL, 1 },
    {"maxT", OPTIONAL, NULL, 2 },
    {"minB", OPTIONAL, NULL, 3 },
    {"maxB", OPTIONAL, NULL, 4 },
    {"test", OPTIONAL, NULL, 5 },
    {"numT", OPTIONAL, NULL, 6 },
    {"tm", OPTIONAL, NULL, 7 },
    {"ct", OPTIONAL, NULL, 8 },
    {"n", OPTIONAL, NULL, 9 },
    {0,0,0,0}
  };

  while (1) {
    int option_index = 0;
    int c =  getopt_long( argc, argv, "", long_options, &option_index );
    if ( c == -1) break;
    switch( c ) {
    case 0:
      args->numIterations = atoi(optarg);
      break;
    case 1:
      args->minNumThreads = atoi(optarg);
      break;
    case 2:
      args->maxNumThreads = atoi(optarg);
      break;
    case 3:
      args->minBuffSize = atoi(optarg);
      break;
    case 4:
      args->maxBuffSize = atoi(optarg);
      break;
    case 5:
      args->test = atoi(optarg);
      break;
    case 6:
      args->numThreads = atoi(optarg);
      break;
    case 7:
      args->threadModel = MPI_THREAD_MULTIPLE;
      break;
    case 8:
      args->compTime = atof(optarg);
      break;
    case 9:
      args->noise = atof(optarg);
      break;
    default:
      return -1;
    }
  }
  return 0;
}

std::string getThreadModelStr(int model)
{
  switch(model) {
  case MPI_THREAD_MULTIPLE: return "Multiple";
  case MPI_THREAD_SERIALIZED: return "Serial";
  }
  assert(0);

  return "NULL";
}
