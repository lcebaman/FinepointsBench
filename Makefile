
#MPI=openmpi
MPI=mpich

FLAGS= #-fopenmp -Wall -O2 -std=gnu++11 
ifeq ($(MPI), openmpi)

CC=mpicc
CXX=mpiCC

else ifeq ($(MPI), mpich)

CC=cc
CXX=CC
INC= -I/lus/scratch/harveyr/mpt/mpich-cray/mpt_rma_install/include
LIBS=-lm -L/lus/scratch/harveyr/mpt/mpich-cray/mpt_rma_install/lib -lmpich_cray -lmpichcxx_cray
FLAGS += -DMPICH -DVERIFY

else
$(error bad mpi distro)
endif 

CPPFLAGS=-I. $(INC)
CXXFLAGS=$(FLAGS)
CFLAGS=$(FLAGS)

SRCS=mpiBase.cc mpiPart.cc \
	mpiBaseTest1.cc \
	mpiBaseTest11.cc \
	mpiBaseTest2.cc \
	mpiBaseTest21.cc \
	mpiBaseTest3.cc \
	mpiBaseTest31.cc \
	mpiBaseTest31t.cc \
	mpiBaseTest32.cc \
	mpiBaseTest5.cc  \
	mpiBaseTestOdd.cc	

OBJS=$(SRCS:.cc=.o)


EXE = partTest.$(MPI) 
all: $(EXE) 

$(EXE): $(OBJS)
	$(CXX) $(CPPFLAGS) $(LIBS) $(CFLAGS) $^  -o $@

clean:
	rm -rf $(OBJS) $(EXE) 
