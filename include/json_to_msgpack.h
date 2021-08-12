#ifndef __JSON_TO_MSGPACK__
#define __JSON_TO_MSGPACK__

void *json_file_to_msgpack_binary(int, size_t *);
void *json_string_to_msgpack_binary(char *, size_t *);

#endif