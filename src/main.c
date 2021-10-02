#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "json_to_msgpack.h"

#define BUFFER_LEN 1024

const char execute[] = "json2msgpack";

static inline void usage()
{
	printf("Usage: %s [FILE] ...\n", execute);
	printf("%s convert json file or standard input json format to msgpack formate in standard output.\n\n", execute);
	printf("Arguments:\n");
	printf("\t-h/--help	display Usage page\n\n");
}

int main(int argc, char *argv[])
{
	size_t size = 0;
	void *output = NULL;

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (*argv[i] == '-')
			{
				if ((strcmp(argv[i] + 1, "h") == 0) || (strcmp(argv[i] + 1, "-help") == 0))
				{
					usage();
					break;
				}
			}
			else
			{
				int fd;
				if ((fd = open(argv[i], O_RDONLY)) < 0)
				{
					fprintf(stderr, "%s: No such file \"%s\".\n", execute, argv[i]);
				}
				else
				{
					if (output = json_file_to_msgpack_binary(fd, &size))
					{
						fwrite(output, size, 1, stdout);
						free(output);
					}

					close(fd);
				}
			}
		}
	}
	else
	{
		//read from stdin
		char buffer[BUFFER_LEN];
		memset(buffer, 0, BUFFER_LEN * sizeof(char));

		if (fread(buffer, sizeof(char), BUFFER_LEN - 1, stdin) > 0)
		{
			if (output = json_string_to_msgpack_binary(buffer, &size))
			{
				fwrite(output, size, 1, stdout);
				free(output);
			}
		}
	}

	return 0;
}
