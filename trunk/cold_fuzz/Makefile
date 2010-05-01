
GTK_LIBS = `pkg-config gtk+-2.0 --libs`
GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags`
GTHREAD = `pkg-config gthread-2.0 --cflags --libs`
GTHREAD_LIBS = `pkg-config gthread-2.0 --libs`

C_FLAGS = -c -Wall -g
CC = gcc

BUILD_DIR = build/
SRC_DIR = src/

cold_fuzz : main results options thread interface fuzzer error network
	$(CC) -o cold_fuzz $(GTK_LIBS) $(GTHREAD_LIBS) \
	$(BUILD_DIR)main.o $(BUILD_DIR)results.o $(BUILD_DIR)options.o $(BUILD_DIR)thread.o \
	$(BUILD_DIR)interface.o $(BUILD_DIR)fuzzer.o $(BUILD_DIR)error.o $(BUILD_DIR)network.o

main : $(SRC_DIR)main.c
	$(CC) $(C_FLAGS) $(SRC_DIR)main.c $(GTK_CFLAGS) -o $(BUILD_DIR)main.o

options : src/options.c
	$(CC) $(C_FLAGS) $(SRC_DIR)options.c $(GTK_CFLAGS) -o $(BUILD_DIR)options.o

thread : src/thread.c
	$(CC) $(C_FLAGS) $(SRC_DIR)thread.c -o $(BUILD_DIR)thread.o

interface : src/interface.c
	$(CC) $(C_FLAGS) $(SRC_DIR)interface.c $(GTK_CFLAGS) -o $(BUILD_DIR)interface.o

results : src/results.c
	$(CC) $(C_FLAGS) $(SRC_DIR)results.c $(GTK_CFLAGS) -o $(BUILD_DIR)results.o

fuzzer : src/fuzzer.c
	$(CC) $(C_FLAGS) $(SRC_DIR)fuzzer.c -o $(BUILD_DIR)fuzzer.o

error : src/error.c
	$(CC) $(C_FLAGS) $(SRC_DIR)error.c -o $(BUILD_DIR)error.o

network : src/network.c
	$(CC) $(C_FLAGS) $(SRC_DIR)network.c $(GTK_CFLAGS) -o $(BUILD_DIR)network.o

clean :
	rm build/*
	rm cold_fuzz
