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
double_buff_queue_OBJS = ${BUILD_DIR}/double_buff_queue_test.o

# bin
${BIN_DIR}/double_buff_queue_test : ${INCLUDE_DIR}/double_buff_queue.h ${double_buff_queue_OBJS}
	${LD} ${LDFLAGS} ${double_buff_queue_OBJS} -o $@

# testing
${BUILD_DIR}/double_buff_queue_test.o : ${TESTING_DIR}/double_buff_queue_test.cpp ${INCLUDE_DIR}/double_buff_queue.h
	${CC} ${CFLAGS} $< -o $@

clean : ${INCLUDE_DIR}
	rm -f ${BIN_DIR}/*
	rm -f ${BUILD_DIR}/*
