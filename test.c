#include <stdio.h>
#include "json2msgpack.h"

int main(int argc, char *argv[])
{
	json_object *input = json_object_from_file(argv[1]);
	json2msgpack(input, stdout);

	free(input);
	return 0;
}