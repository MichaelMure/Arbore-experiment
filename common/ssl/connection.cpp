
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connection.h"

Connection::Connection(int _fd) : fd(_fd),
			read_buf(NULL),
			read_buf_size(0)
{
}

// Read "size" octets from the read buffer
// returns true and buf if enough datas can be read
// returns false when not enough data is available
bool Connection::Read(char **buf, size_t size)
{
	// Fill the buffer
	ReadToBuf();

	if(read_buf_size < size)
		return false;

	*buf = (char*)malloc(size);
	memcpy(*buf, read_buf, size);

	// The buffer has been totally read
	if(size == read_buf_size)
	{
		free(read_buf);
		read_buf_size = 0;
		return true;
	}

	// Remove read data from the buffer
	read_buf_size -= size;
	char* new_buf = (char*)malloc(read_buf_size);
	memcpy(new_buf, read_buf + size, read_buf_size);
	free(read_buf);
	read_buf = new_buf;
	return true;
}

