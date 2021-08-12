#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "json_to_msgpack.h"

int main(int argc, char *argv[])
{
    size_t size = 0;
    //int fd = open(argv[1], O_RDONLY);
    //void *output = json_file_to_msgpack_binary(fd, &size);

    char buffer[1024] = {0};
    fread(buffer, sizeof(char), 1024 - 1, stdin);
    void *output = json_string_to_msgpack_binary(buffer, &size);

    fwrite(output, size, 1, stdout);
    return 0;
}