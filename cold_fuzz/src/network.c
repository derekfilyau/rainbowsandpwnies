#include "network.h"


char * storage_units (int size)
{

	memset(storage_units_tmp, 0, 32);
	if (size > 1024*1024*10)
		snprintf(storage_units_tmp, 32, "%d mb", size/1024/1024);
	else if (size > 1024*10)
		snprintf(storage_units_tmp, 32, "%d kb", size/1024);
	else
		snprintf(storage_units_tmp, 32, "%d bytes", size);
	
	return storage_units_tmp;

}



int network_connect (char * hostname, char * port, struct network_info ** info_arg)
{
	
	int status;
	struct addrinfo hints;
	struct addrinfo * results;
	struct network_info * info;
	
	
	char ip_address[30];
	
	*info_arg = (struct network_info *) malloc(sizeof(struct network_info));
	if (*info_arg == NULL)
		return FUZZER_ERROR_INFO_MALLOC_FAIL;
	
	info = *info_arg;
	
	memset(info->debug_buffer, 0, DEBUG_BUFFER_SIZE);
		
	/*
	* RESOLVE HOSTNAME
	*/
	
	memset(&hints, 0, sizeof(struct addrinfo));
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	status = getaddrinfo(hostname, port, &hints, &results);
	if (status != 0)
	{
		snprintf(FUZZER_ERROR_CUSTOM, FUZZER_ERROR_CUSTOM_MAX_LENGTH, "getaddrinfo: %s\n", gai_strerror(status));
		return FUZZER_ERROR_GETADDR_INFO;
	}
	
	memcpy(&(info->addr), results, sizeof (struct addrinfo));
	freeaddrinfo(results);
	
	inet_ntop(info->addr.ai_family, (void *) &((struct sockaddr_in *) info->addr.ai_addr)->sin_addr, ip_address, 30);
	
	snprintf(info->debug_buffer, DEBUG_BUFFER_SIZE, "%s:%s ... ", ip_address, port);
	
	/*
	* CREATE SOCKET
	*/
	
	info->sock = socket(info->addr.ai_family, info->addr.ai_socktype, info->addr.ai_protocol);
	if (info->sock == -1)
	{
		return FUZZER_ERROR_SOCKET_CREATE;
	}
	
	/*
	* CONNECT
	*/
	status = connect(info->sock, info->addr.ai_addr, info->addr.ai_addrlen);
	
	if (status == -1)
	{
		if (errno == ECONNREFUSED)
			return FUZZER_ERROR_CONNECTION_REFUSED;
		else if (errno == EADDRNOTAVAIL)
			return FUZZER_ERROR_ADDRESS_NOT_AVAILABLE;
		else if (errno == ENETUNREACH)
			return FUZZER_ERROR_NETWORK_UNREACHABLE;
		else if (errno == EAFNOSUPPORT)
			return FUZZER_ERROR_INVALID_FAMILY;
		else if (errno == EHOSTUNREACH)
			return FUZZER_ERROR_HOST_UNREACHABLE;
		else
		{
			printf("%d\n", errno);
			return FUZZER_ERROR_CONNECT_UNKNOWN;
		}
	}
	
	strncat(info->debug_buffer, "connected", DEBUG_BUFFER_SIZE);
	
	return 0;
	
}



int network_send (struct network_info * info, unsigned char * data, int data_len)
{

	int data_sent = 0;
	int bytes_sent = 0;
	
	char tmp[128];
	
	while (data_sent < data_len)
	{
		
		bytes_sent = send(info->sock, &(data[data_sent]), data_len - data_sent, 0);
		if (bytes_sent == -1)
		{
			return FUZZER_ERROR_SOCKET_CLOSED_ON_SEND;
		}
		data_sent += bytes_sent;
	}
	
	snprintf(tmp, 128, " - sent %s", storage_units(data_sent));
	strncat(info->debug_buffer, tmp, DEBUG_BUFFER_SIZE);
	
	return 0;
	
}



int network_receive (struct network_info * info)
{
	
	// set non-blocking
	fcntl(info->sock, F_SETFL, O_NONBLOCK);

	int i;
	int bytes_received;
	char buf[4096];
	char tmp[128];
	
	info->bytes_in_buffer = 0;
	
	int fuzzer_buffer_size_writable = FUZZER_BUFFER_SIZE - 1;
	
	memset(info->buffer, 0, FUZZER_BUFFER_SIZE);
	
	for (i = 0; i < FUZZER_SLEEP_TIMES; i++)
	{
		bytes_received = recv (info->sock, buf, 4096, 0);
		if (bytes_received == 0)
			return 0;
		else if (bytes_received == -1)
		{
			if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
				usleep(FUZZER_SLEEP_MICROSECONDS);
			else
				return FUZZER_ERROR_RECV_UNKNOWN;
		}
		else if (bytes_received + info->bytes_in_buffer > fuzzer_buffer_size_writable)
		{
			memcpy(&(info->buffer[info->bytes_in_buffer]), buf, fuzzer_buffer_size_writable - info->bytes_in_buffer);
			info->bytes_in_buffer += fuzzer_buffer_size_writable - info->bytes_in_buffer;
		}
		else
		{
			memcpy(&(info->buffer[info->bytes_in_buffer]), buf, bytes_received);
			info->bytes_in_buffer += bytes_received;
		}
	}
	
	snprintf(tmp, 128, " - received %d bytes", info->bytes_in_buffer);
	strncat(info->debug_buffer, tmp, DEBUG_BUFFER_SIZE);
	strncat(info->debug_buffer, "\n", DEBUG_BUFFER_SIZE);
	
	return 0;
	
}



void network_close (struct network_info * info)
{
	if (info != NULL)
	{
		if (info->sock != -1)
			close(info->sock);
		free(info);
	}
}
	
