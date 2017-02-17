#!/bin/sh
TOPDIR=../..

CFLAGS=-g -fno-omit-frame-pointer
INCLUDE=-I${TOPDIR}/include
LIB=-L${TOPDIR}/lib64 -lrocketmq -lz -lrt -lpthread
TARGET=Producer PullConsumer PushConsumer
DEPEND_OBJ=


LOCAL_SRC += $(sort $(wildcard *.cpp *.cc *.c))
LOCAL_OBJ += $(patsubst %.cpp,%.o, $(patsubst %.cc,%.o, $(patsubst %.c,%.o, $(LOCAL_SRC))))

all:${LOCAL_OBJ} ${TARGET}
	@echo "build succ!"

clean:
	rm -vf ${TARGET} *.o
	@echo "clean succ!"

Producer:Producer.o ${DEPEND_OBJ}
	$(CXX) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIB)

PullConsumer:PullConsumer.o ${DEPEND_OBJ}
	$(CXX) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIB)

PushConsumer:PushConsumer.o ${DEPEND_OBJ}
	$(CXX) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIB)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

%.o: %.cc
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
