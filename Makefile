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
DOUBLE_BUFF_QUEUE_OBJS = ${BUILD_DIR}/double_buff_queue_test.o
THREAD_POOLOBJS = ${BUILD_DIR}/thread_pool_test.o
TESTING_BIN = ${BIN_DIR}/thread_pool_test ${BIN_DIR}/double_buff_queue_test


all : ${TESTING_BIN}

# bin
${BIN_DIR}/double_buff_queue_test : ${DOUBLE_BUFF_QUEUE_OBJS}
	${LD} ${LDFLAGS} ${DOUBLE_BUFF_QUEUE_OBJS} -o $@
${BIN_DIR}/thread_pool_test : ${THREAD_POOLOBJS}
	${LD} ${LDFLAGS} ${THREAD_POOLOBJS} -o $@

# testing
${BUILD_DIR}/double_buff_queue_test.o : ${TESTING_DIR}/double_buff_queue_test.cpp ${INCLUDE_DIR}/double_buff_queue.h
	${CC} ${CFLAGS} $< -o $@
${BUILD_DIR}/thread_pool_test.o : ${TESTING_DIR}/thread_pool_test.cpp ${INCLUDE_DIR}/thread_pool.h
	${CC} ${CFLAGS} $< -o $@

clean : ${INCLUDE_DIR}
	rm -f ${BIN_DIR}/*
	rm -f ${BUILD_DIR}/*
