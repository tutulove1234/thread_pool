CC = g++ 
LDFLAG = -lpthread
CFLAG = -c -std=c++14
TARGET = thread_pool
CPP = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(CPP))

.PHONY : clean

all: ${TARGET}

${TARGET}: ${OBJ}
	${CC} -o $@ $< ${LDFLAG}
${OBJ}: ${CPP}
	${CC} ${CFLAG} $^

clean :
	@rm -f ${TARGET} ${OBJ}

