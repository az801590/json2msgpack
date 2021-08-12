#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "json_to_msgpack.h"

int main(int argc, char *argv[])
{
    size_t size = 0;
    
    int fd = open(argv[1], O_RDONLY);
    void *output = json_file_to_msgpack_binary(fd, &size);

    //char buffer[1024] = {0};
    //fread(buffer, sizeof(char), 1024 - 1, stdin);
    //void *output = json_string_to_msgpack_binary(buffer, &size);

    fwrite(output, size, 1, stdout);
    return 0;
}

/*
size_t msgpack_object_data_set(msgpack_object *msgp, const void *buff, size_t buffSize, size_t buffLen, msgpack_type type)
{
    if(!msgpack_object_data_alloc(msgp, buffSize))
    {
        return 0;
    }

    memcpy(msgp->data, buff, buffSize);
    msgp->length = buffLen;
    msgp->type = type;

    return msgp->size;
}

json_object_to_msgpack_object
{
...
	if (parent)
	{
		if (parent->data != NULL)
		{
			out->next = parent->data;
			out->last = ((msgpack_object *)parent->data)->last;
			((msgpack_object *)(parent->data))->last->next = out;
			((msgpack_object *)parent->data)->last = out;
		}

		parent->data = out;
	}
...
}
*/