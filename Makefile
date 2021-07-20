BUILD_DIR = ./build
BIN_DIR = ./bin
INCLUDE_DIR = ./include
TESTING_DIR = ./testing
CC = g++
LD = g++
SO = -lpthread
LIB = -I ${INCLUDE_DIR}
CFLAGS = -c ${LIB}
LDFLAGS = ${SO} -L ./lib 
SAFE_QUEUE_OBJS = ${BUILD_DIR}/safe_queue_test.o

# bin
${BIN_DIR}/safe_queue_test : ${INCLUDE_DIR}/safe_queue.h ${SAFE_QUEUE_OBJS}
	${LD} ${LDFLAGS} ${SAFE_QUEUE_OBJS} -o $@

# testing
${BUILD_DIR}/safe_queue_test.o : ${TESTING_DIR}/safe_queue_test.cpp ${INCLUDE_DIR}/safe_queue.h
	${CC} ${CFLAGS} $< -o $@

clean : ${INCLUDE_DIR}
	rm -f ${BIN_DIR}/*
	rm -f ${BUILD_DIR}/*
