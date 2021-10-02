#ifndef __MSGPACK_OBJECT__
#define __MSGPACK_OBJECT__

#include <stddef.h>

typedef enum
{
    MSGPACK_TYPE_NIL,
    MSGPACK_TYPE_FALSE,
    MSGPACK_TYPE_TRUE,
    MSGPACK_TYPE_SIGNED_INT,
    MSGPACK_TYPE_UNSIGNED_INT,
    MSGPACK_TYPE_FLOAT,
    MSGPACK_TYPE_DOUBLE,
    MSGPACK_TYPE_STR,
    MSGPACK_TYPE_BIN,
    MSGPACK_TYPE_ARR,
    MSGPACK_TYPE_MAP,
    MSGPACK_TYPE_EXT,
    MSGPACK_TYPE_TIME,
    MSGPACK_TYPE_NEVER_USED
} msgpack_type;

/*
typedef enum
{
    MSGPACK_INT_FIXINT,
    MSGPACK_INT_N_FIXINT,
    MSGPACK_INT_UINT8,
    MSGPACK_INT_UINT16,
    MSGPACK_INT_UINT32,
    MSGPACK_INT_UINT64,
    MSGPACK_INT_INT8,
    MSGPACK_INT_INT16,
    MSGPACK_INT_INT32,
    MSGPACK_INT_INT64
} msgpack_int_type;
*/

typedef struct MSGPACK_OBJECT
{
    void *data;
    size_t size;
    size_t length;
    msgpack_type type;

    struct MSGPACK_OBJECT *next;
    struct MSGPACK_OBJECT *last;
} msgpack_object;

msgpack_object *msgpack_object_create();
size_t set_msgpack_object_data(msgpack_object *,const void *, size_t, size_t, msgpack_type);
msgpack_object *add_msgpack_object_to_list(msgpack_object *, msgpack_object *);
uint8_t get_msgpack_object_format(msgpack_object *);
void msgpack_object_free(msgpack_object *);

#endif