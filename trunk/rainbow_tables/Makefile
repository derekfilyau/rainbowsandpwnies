C_FLAGS = -c -Wall -O3
CC = gcc

OPENMP = -fopenmp

BUILD_DIR = bin/
SRC_DIR = src/

rainbow_tables : rc rc2 rmm ru rs rsm rss rsc rsp

rc : $(SRC_DIR)rc.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rc.c -o $(BUILD_DIR)rc

rc2 : $(SRC_DIR)rc2.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rc2.c -o $(BUILD_DIR)rc2

rmm : $(SRC_DIR)rmm.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rmm.c $(OPENMP) -o $(BUILD_DIR)rmm

ru : $(SRC_DIR)ru.c
	$(CC) $(C_FLAGS) $(SRC_DIR)ru.c -o $(BUILD_DIR)ru

rs : $(SRC_DIR)rs.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rs.c -o $(BUILD_DIR)rs

rsm : $(SRC_DIR)rsm.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rsm.c -o $(BUILD_DIR)rsm

rss : $(SRC_DIR)rss.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rss.c -o $(BUILD_DIR)rss

rsc : $(SRC_DIR)rsc.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rsc.c -o $(BUILD_DIR)rsc

rsp : $(SRC_DIR)rsp.c
	$(CC) $(C_FLAGS) $(SRC_DIR)rsp.c -o $(BUILD_DIR)rsp

clean :
	rm bin/*
