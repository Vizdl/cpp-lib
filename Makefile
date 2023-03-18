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
DOUBLE_BUFF_QUEUE_OBJS = ${BUILD_DIR}/double_buff_queue_test.o
THREAD_POOL_OBJS = ${BUILD_DIR}/thread_pool_test.o
SPIN_LOCK_OBJS = ${BUILD_DIR}/spin_lock_test.o
RW_LOCK_OBJS = ${BUILD_DIR}/rw_lock_test.o
ATOMIC_OBJS = ${BUILD_DIR}/atomic_test.o
SAFE_QUEUE_OBJS = ${BUILD_DIR}/safe_queue_test.o
LIST_OBJS = ${BUILD_DIR}/list_test.o
HASHMAP_OBJS = ${BUILD_DIR}/hashmap_test.o
REFCOUNT_OBJS = ${BUILD_DIR}/refcount_test.o

TESTING_BIN = ${BIN_DIR}/thread_pool_test ${BIN_DIR}/double_buff_queue_test ${BIN_DIR}/spin_lock_test \
	${BIN_DIR}/rw_lock_test ${BIN_DIR}/atomic_test ${BIN_DIR}/safe_queue_test ${BIN_DIR}/list_test \
	${BIN_DIR}/hashmap_test ${BIN_DIR}/refcount_test

all : ${TESTING_BIN}

# bin
${BIN_DIR}/double_buff_queue_test : ${DOUBLE_BUFF_QUEUE_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/thread_pool_test : ${THREAD_POOL_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/spin_lock_test : ${SPIN_LOCK_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/rw_lock_test : ${RW_LOCK_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/atomic_test : ${ATOMIC_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/safe_queue_test : ${SAFE_QUEUE_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/list_test : ${LIST_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/hashmap_test : ${HASHMAP_OBJS}
	${LD} $< -o $@ ${LDFLAGS}
${BIN_DIR}/refcount_test : ${REFCOUNT_OBJS}
	${LD} $< -o $@ ${LDFLAGS}

# testing
${DOUBLE_BUFF_QUEUE_OBJS} : ${TESTING_DIR}/double_buff_queue_test.cpp ${INCLUDE_DIR}/double_buff_queue.h
	${CC} ${CFLAGS} $< -o $@
${THREAD_POOL_OBJS} : ${TESTING_DIR}/thread_pool_test.cpp ${INCLUDE_DIR}/thread_pool.h
	${CC} ${CFLAGS} $< -o $@
${SPIN_LOCK_OBJS} : ${TESTING_DIR}/spin_lock_test.cpp ${INCLUDE_DIR}/spin_lock.h
	${CC} ${CFLAGS} $< -o $@
${RW_LOCK_OBJS} : ${TESTING_DIR}/rw_lock_test.cpp ${INCLUDE_DIR}/rw_lock.h
	${CC} ${CFLAGS} $< -o $@
${BUILD_DIR}/atomic_test.o : ${TESTING_DIR}/atomic_test.c ${INCLUDE_DIR}/atomic.h
	${CC} ${CFLAGS} $< -o $@
${SAFE_QUEUE_OBJS} : ${TESTING_DIR}/safe_queue_test.cpp ${INCLUDE_DIR}/safe_queue.h
	${CC} ${CFLAGS} $< -o $@
${LIST_OBJS} : ${TESTING_DIR}/list_test.c ${INCLUDE_DIR}/list.h
	${CC} ${CFLAGS} $< -o $@
${HASHMAP_OBJS} : ${TESTING_DIR}/hashmap_test.c ${INCLUDE_DIR}/hashmap.h
	${CC} ${CFLAGS} $< -o $@
${REFCOUNT_OBJS} : ${TESTING_DIR}/refcount_test.c
	${CC} ${CFLAGS} $< -o $@

clean : ${INCLUDE_DIR}
	rm -f ${BIN_DIR}/*
	rm -f ${BUILD_DIR}/*
