#ifndef __MSGPACK_WRITE_BUFF__
#define __MSGPACK_WRITE_BUFF__

#include <stddef.h>

typedef struct 
{
    void *data;
    size_t offset;
    size_t size;
} write_buff;

write_buff *get_msgpack_write_buff(write_buff *, msgpack_object *);
void write_buff_free(write_buff *);

#endif