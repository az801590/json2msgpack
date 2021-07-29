#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <json.h>

#include "json2msgpack.h"

#define BUFFER_LEN 1024

const char execute[] = "json2msgpack";

static inline void usage()
{
	fputs("\
Usage: json2msgpack [FILE] ...\n\
json2msgpack convert json file or standard input json format to msgpack formate in standard output.\n\
\n\
Arguments:\n\
  -h/--help	display Usage page\n\
\n",
		  stdout);
}

int main(int argc, char *argv[])
{
	json_object *input;

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
					if (input = json_object_from_fd(fd))
					{
						json2msgpack(input, stdout);
						free(input);
					}
					else
					{
						fprintf(stderr, "%s: Invalid input.\n", execute);
						close(fd);
						break;
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
			if (input = json_tokener_parse(buffer))
			{
				json2msgpack(input, stdout);
				free(input);
			}
			else
			{
				fprintf(stderr, "%s: Invalid input.\n", execute);
			}
		}
	}

	return 0;
}
