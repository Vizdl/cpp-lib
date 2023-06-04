BUILD_DIR = ./build
BIN_DIR = ./bin
INCLUDE_DIR = ./include
TESTING_DIR = ./testing
CC = gcc
LD = g++
SO = -lpthread
LIB = -I ${INCLUDE_DIR}
CFLAGS = -c ${LIB}
LDFLAGS = ${SO} -L ./lib 

TESTING_BIN = ${BIN_DIR}/thread_pool_test ${BIN_DIR}/double_buff_queue_test ${BIN_DIR}/spin_lock_test \
	${BIN_DIR}/rw_lock_test ${BIN_DIR}/atomic_test ${BIN_DIR}/safe_queue_test ${BIN_DIR}/list_test \
	${BIN_DIR}/hashmap_test ${BIN_DIR}/refcount_test

all : mkdir ${TESTING_BIN}

mkdir :
	mkdir bin -p && mkdir build -p

# bin
${BIN_DIR}/% : ${BUILD_DIR}/%.o
	${LD} $< -o $@ ${LDFLAGS}

# testing
${BUILD_DIR}/%.o : ${TESTING_DIR}/%.cpp
	${CC} ${CFLAGS} $< -o $@
${BUILD_DIR}/%.o : ${TESTING_DIR}/%.c
	${CC} ${CFLAGS} $< -o $@

clean : ${INCLUDE_DIR}
	rm -f ${BIN_DIR}/*
	rm -f ${BUILD_DIR}/*
