#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "msgpack_object.h"
#include "msgpack_write_buff.h"

static void toBigEndian(void *data, size_t size)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if (size == 2)
    {
        *(uint16_t *)data = __bswap_16(*(uint16_t *)data);
    }
    else if (size == 4)
    {
        *(uint32_t *)data = __bswap_32(*(uint32_t *)data);
    }
    else if (size == 8)
    {
        *(uint64_t *)data = __bswap_64(*(uint64_t *)data);
    }
#endif
}

write_buff *write_buff_create()
{
    write_buff *tmp = calloc(1, sizeof(write_buff));
    if (tmp)
    {
        tmp->data = calloc(8, sizeof(uint8_t));
        if (tmp->data)
        {
            tmp->size = 8;
            tmp->offset = 0;
            return tmp;
        }
        else
        {
            free(tmp);
        }
    }
    return NULL;
}

int write_buff_data_expand(write_buff *buff, size_t inputSize)
{

    inputSize = inputSize < 8 ? 8 : inputSize + 8;

    if ((buff->size - buff->offset) < inputSize)
    {
        int newSize = buff->size * 2;
        void *tmp = NULL;

        while ((newSize - buff->offset) < inputSize)
        {
            newSize *= 2;
        }

        tmp = realloc(buff->data, newSize);

        if (!tmp)
        {
            return 0;
        }

        buff->data = tmp;
        buff->size = newSize;
    }

    return 1;
}

int set_write_buff_data(write_buff *buff, msgpack_object *obj)
{
    if (!write_buff_data_expand(buff, obj->size))
    {
        return 0;
    }

    //add format
    uint8_t type = get_msgpack_object_format(obj);
    if (type && type != MSGPACK_FORMAT_NEVER_USED)
    {
        *((uint8_t *)(buff->data + buff->offset)) = type;
        buff->offset += sizeof(type);
    }

    //add length
    if (
        (obj->type == MSGPACK_TYPE_BIN) ||
        (obj->type == MSGPACK_TYPE_STR && obj->length > 31) ||
        (obj->length > 15 && (obj->type == MSGPACK_TYPE_ARR || obj->type == MSGPACK_TYPE_MAP)))
    {
        if ((obj->length < (1 << 8)) && (obj->type == MSGPACK_TYPE_BIN || obj->type == MSGPACK_TYPE_STR))
        {
            uint8_t tmpLen = obj->length;
            *((uint8_t *)(buff->data + buff->offset)) = tmpLen;
            buff->offset += sizeof(tmpLen);
        }
        else if (obj->length < (1 << 16))
        {
            uint16_t tmpLen = obj->length;
            toBigEndian(&tmpLen, sizeof(tmpLen));
            memcpy(buff->data + buff->offset, &tmpLen, sizeof(tmpLen));
            buff->offset += sizeof(tmpLen);
        }
        else
        {
            //obj->length <= (4294967295)
            uint32_t tmpLen = obj->length;
            toBigEndian(&tmpLen, sizeof(tmpLen));
            memcpy(buff->data + buff->offset, &tmpLen, sizeof(tmpLen));
            buff->offset += sizeof(tmpLen);
        }
    }

    //add data
    if (obj->size)
    {
        if (
            obj->type == MSGPACK_TYPE_SIGNED_INT || obj->type == MSGPACK_TYPE_UNSIGNED_INT ||
            obj->type == MSGPACK_TYPE_FLOAT || obj->type == MSGPACK_TYPE_DOUBLE)
        {
            toBigEndian(obj->data, obj->size);
        }

        memcpy(buff->data + buff->offset, obj->data, obj->size);
        buff->offset += obj->size;
    }
    return 1;
}

void write_buff_free(write_buff *buff)
{
    if (buff)
    {
        /*
        if (buff->data)
        {
            free(buff->data);
        }
        */
        free(buff);
    }
}

void write_buff_data_free(write_buff *buff)
{
    if(buff && buff->data)
    {
        free(buff->data);
    }
}

write_buff *get_msgpack_write_buff(write_buff *output, msgpack_object *obj)
{
    if (!obj)
    {
        return output;
    }

    if (!output)
    {
        output = write_buff_create();
    }

    msgpack_object *ptr = obj;
    do
    {
        if (!set_write_buff_data(output, ptr))
        {
            return NULL;
        }

        if ((ptr->type == MSGPACK_TYPE_MAP || ptr->type == MSGPACK_TYPE_ARR) && ptr->data)
        {
            get_msgpack_write_buff(output, ptr->data);
        }

        ptr = ptr->next;
    } while (ptr && ptr != obj);

    return output;
}