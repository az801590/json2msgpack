#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "msgpack_object.h"

int msgpack_object_init(msgpack_object *msgp)
{
    if(!msgp)
    {
        return 0;
    }
    
    msgp->data = NULL;
    msgp->size = 0;
    msgp->length = 0;
    msgp->type = MSGPACK_TYPE_NEVER_USED;

    msgp->next = msgp;
    msgp->last = msgp;

    return 1;
}

msgpack_object *msgpack_object_create()
{
    msgpack_object *new = calloc(1, sizeof(msgpack_object));
    if (!new || !msgpack_object_init(new))
    {
        return NULL;
    }

    return new;
}

int msgpack_object_data_alloc(msgpack_object *obj, size_t size)
{
    if (!obj)
    {
        return 0;
    }

    if (size > 0)
    {
        if (obj->data)
        {
            if (obj->size < size)
            {
                void *tmp = realloc(obj->data, size);
                if (!tmp)
                {
                    return 0;
                }
                else
                {
                    obj->data = tmp;
                    obj->size = size;
                }
            }
        }
        else
        {
            void *tmp = calloc(1, size);
            if(!tmp)
            {
                return 0;
            }
            else
            {
                obj->data = tmp;
                obj->size = size;
            }
        }
    }

    return 1;
}

size_t set_msgpack_object_data(msgpack_object *msgp, const void *buff, size_t buffSize, size_t buffLen, msgpack_type type)
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

msgpack_object *add_msgpack_object_to_list(msgpack_object *head, msgpack_object *new)
{
    if (head)
    {
        new->next = head;
        new->last = head->last;

        head->last->next = new;
        head->last = new;

        return head;
    }
    else
    {
        return new;
    }
}

int add_msgpack_pack_to_child(msgpack_object *parent, msgpack_object *new)
{
    if(!parent || !new || (parent->type != MSGPACK_TYPE_MAP) || (parent->type != MSGPACK_TYPE_ARR))
    {
        return 0;
    }

    if(parent->data)
    {
        new->last = ((msgpack_object*)parent->data)->last;
        new->next = parent->data;

        ((msgpack_object*)parent->data)->last->next = new;
        ((msgpack_object*)parent->data)->last = new;
    }
    else
    {
        parent->data = new;
    }

    return 1;
}

void msgpack_object_free(msgpack_object *obj)
{
    if(obj)
    {
        obj->last->next = NULL;

        if(obj->type == MSGPACK_TYPE_ARR || obj->type == MSGPACK_TYPE_MAP)
        {
            msgpack_object_free(obj->data);
        }
        else
        {
            if(obj->data)
            {
                free(obj->data);
            }
        }

        msgpack_object_free(obj->next);
        free(obj);
    }
}

uint8_t get_msgpack_int_format(msgpack_object *obj)
{
    if (obj->type == MSGPACK_TYPE_SIGNED_INT)
    {
        if (obj->size == 1)
        {
            int8_t value = *((int8_t *)obj->data);
            if (value < 0 && value >= -(1 << 5))
            {
                //return (uint8_t)value;
                return 0;
            }

            return 0xd0;
        }
        else if (obj->size == 2)
        {
            return 0xd1;
        }
        else if (obj->size == 4)
        {
            return 0xd2;
        }
        else
        {
            //64-bit big-endian signed integer
            return 0xd3;
        }
    }
    else
    {
        //MSGPACK_TYPE_UNSIGNED_INT
        if (obj->size == 1)
        {
            if (*((uint8_t *)obj->data) < (1 << 7))
            {
                //return *((uint8_t *)obj->data);
                return 0;
            }
            else
            {
                return 0xcc;
            }
        }
        else if (obj->size == 2)
        {
            return 0xcd;
        }
        else if (obj->size == 4)
        {
            return 0xce;
        }
        else
        {
            //64-bit big-endian unsigned integer
            return 0xcf;
        }
    }
}

uint8_t get_msgpack_string_format(msgpack_object *obj)
{
    if (obj->length < (1 << 5))
    {
        return (uint8_t)(0xa0 | obj->length);
    }
    else if (obj->length < (1 << 8))
    {
        return 0xd9;
    }
    else if (obj->length < (1 << 16))
    {
        return 0xda;
    }
    else /*if (obj->length <= (4294967295))*/
    {
        return 0xdb;
    }
    /* greater than (2^32)-1
    else
    {
    }
    */
}

uint8_t get_msgpack_array_format(msgpack_object *obj)
{
    if (obj->length < (1 << 4))
    {
        return (uint8_t)(0x90 | obj->length);
    }
    else if (obj->length < (1 << 16))
    {
        return 0xdc;
    }
    else /*if (obj->length <= (4294967295))*/
    {
        return 0xdd;
    }
    /*
    else
    {
    }
    */
}

uint8_t get_msgpack_map_format(msgpack_object *obj)
{
    if (obj->length < (1 << 5))
    {
        return (uint8_t)(0x80 | obj->length);
    }
    else if (obj->length < (1 << 16))
    {
        return 0xde;
    }
    else /*if (obj->length <= ((4294967295)))*/
    {
        return 0xdf;
    }
    /*
    else
    {
    }
    */
}

uint8_t get_msgpack_object_format(msgpack_object *obj)
{
    switch (obj->type)
    {
        case MSGPACK_TYPE_NIL:
            return 0xc0;
        case MSGPACK_TYPE_FALSE:
            return 0xc2;
        case MSGPACK_TYPE_TRUE:
            return 0xc3;
        case MSGPACK_TYPE_SIGNED_INT:
        case MSGPACK_TYPE_UNSIGNED_INT:
            return get_msgpack_int_format(obj);
        case MSGPACK_TYPE_FLOAT:
            return 0xca;
        case MSGPACK_TYPE_DOUBLE:
            return 0xcb;
        case MSGPACK_TYPE_STR:
            return get_msgpack_string_format(obj);
        case MSGPACK_TYPE_ARR:
            return get_msgpack_array_format(obj);
        case MSGPACK_TYPE_MAP:
            return get_msgpack_map_format(obj);
        case MSGPACK_TYPE_BIN:
        case MSGPACK_TYPE_EXT:
        case MSGPACK_TYPE_TIME:
        case MSGPACK_TYPE_NEVER_USED:
            //unfinished
            return 0xc1;
    }
}