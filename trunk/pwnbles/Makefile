C_FLAGS = -Wall -g
CC = gcc

BUILD_DIR = build/
SRC_DIR = src/

all : r hash_md4
	$(CC) $(C_FLAGS) -o test  $(SRC_DIR)test.c $(BUILD_DIR)md4.o
	$(CC) $(C_FLAGS) -o rmake $(SRC_DIR)rmake.c $(BUILD_DIR)r.o \
	$(BUILD_DIR)hash_md4.o $(BUILD_DIR)md4.o

r : $(SRC_DIR)r.c
	$(CC) -c $(C_FLAGS) $(SRC_DIR)r.c -o $(BUILD_DIR)r.o

hash_md4 : $(SRC_DIR)hash_md4.c md4
	$(CC) -c $(C_FLAGS) $(SRC_DIR)hash_md4.c -o $(BUILD_DIR)hash_md4.o

md4 : $(SRC_DIR)md4.c
	$(CC) -c $(C_FLAGS) $(SRC_DIR)md4.c -o $(BUILD_DIR)md4.o

clean :
	rm build/*
	rm cold_fuzz
