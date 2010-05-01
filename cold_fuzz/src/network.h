#ifndef network_HEADER
#define network_HEADER

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


#include "error.h"
#include "options.h"
//#include "interface.h"


#define DEBUG_NETWORK 1



#define FUZZER_BUFFER_SIZE 8192
#define FUZZER_SLEEP_MICROSECONDS 400*1000 // 0.4 seconds
#define FUZZER_SLEEP_TIMES 10 // 0.4 * 10 = 4 seconds
#define DEBUG_BUFFER_SIZE 256


struct network_info
{
	int sock;
	int bytes_in_buffer;
	struct addrinfo addr;
	char buffer [FUZZER_BUFFER_SIZE];
	char debug_buffer[256];
};



char storage_units_tmp[32];
char * storage_units (int size);
int network_connect (char * hostname, char * port, struct network_info ** info_arg);
int network_send (struct network_info * info, unsigned char * data, int data_len);
int network_receive (struct network_info * info);
void network_close (struct network_info * info);

#endif
