#include "error.h"


char FUZZER_ERROR_CUSTOM[256];
int fuzzer_errno;


void reset_fuzzer_error_custom ()
{
	memset(FUZZER_ERROR_CUSTOM, 0, FUZZER_ERROR_CUSTOM_MAX_LENGTH);
}


char * fuzzer_error_string (int error)
{

	switch (error)
	{
			
		case FUZZER_ERROR_GETADDR_INFO :
			return FUZZER_ERROR_CUSTOM;
		case FUZZER_ERROR_INFO_MALLOC_FAIL :
			return "ERROR ALLOCATING MEMORY FOR network_info";
		case FUZZER_ERROR_SOCKET_CREATE :
			return "FAILED TO CREATE SOCKET";
		case FUZZER_ERROR_CONNECTION_REFUSED :
			return "CONNECTION REFUSED";
		case FUZZER_ERROR_ADDRESS_NOT_AVAILABLE :
			return "ADDRESS NOT AVAILABLE";
		case FUZZER_ERROR_NETWORK_UNREACHABLE :
			return "NETWORK UNREACHABLE";
		case FUZZER_ERROR_INVALID_FAMILY :
			return "ADDRESS NOT VALID FOR SOCKET FAMILY";
		case FUZZER_ERROR_CONNECT_UNKNOWN :
			return "UNKNOWN ERROR ON CONNECT";
		case FUZZER_ERROR_SOCKET_CLOSED_ON_SEND :
			return "SOCKET CLOSED ON SEND";
		case FUZZER_ERROR_RECV_UNKNOWN :
			return "UNKNOWN ERROR ON RECV";
		case FUZZER_ERROR_HOST_UNREACHABLE :
			return "HOST UNREACHABLE";
		default :
			memset(FUZZER_ERROR_CUSTOM, 0, 256);
			snprintf(FUZZER_ERROR_CUSTOM, 256, "UNKNOWN ERROR %d\n", error);
			return FUZZER_ERROR_CUSTOM;
	}

}
