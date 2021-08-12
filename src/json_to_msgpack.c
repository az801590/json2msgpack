#include <stdio.h>
#include <stdint.h>
#include <json.h>

#include "msgpack_object.h"
#include "msgpack_write_buff.h"
#include "json_to_msgpack.h"

static inline msgpack_type get_int_type(int64_t value)
{
	if (value >= 0)
	{
		return MSGPACK_TYPE_UNSIGNED_INT;
	}
	else
	{
		return MSGPACK_TYPE_SIGNED_INT;
	}
}

static size_t get_int_size(int64_t value)
{
	if (
		(value >= 0 && value < (1 << 8)) ||
		(value < 0 && value >= -(1 << 8)))
	{
		return 1;
	}
	else if (
		(value >= 0 && value < (1 << 16)) ||
		(value < 0 && value >= -(1 << 16)))
	{
		return 2;
	}
	else if (
		(value >= 0 && value < 4294967296) ||
		(value < 0 && value >= -4294967296))
	{
		return 4;
	}
	else
	{
		return 8;
	}
}

msgpack_object *json_object_to_msgpack_object(msgpack_object *parent, json_object *input)
{
	if (!input)
	{
		return NULL;
	}

	msgpack_object *out = msgpack_object_create();
	json_type type = json_object_get_type(input);

	if (type == json_type_object)
	{
		msgpack_object_data_set(out, NULL, 0, json_object_object_length(input), MSGPACK_TYPE_MAP);
	}
	else if (type == json_type_array)
	{
		msgpack_object_data_set(out, NULL, 0, json_object_array_length(input), MSGPACK_TYPE_ARR);
	}
	else if (type == json_type_string)
	{
		msgpack_object_data_set(out, json_object_get_string(input), json_object_get_string_len(input), json_object_get_string_len(input), MSGPACK_TYPE_STR);
	}
	else if (type == json_type_null)
	{
		msgpack_object_data_set(out, NULL, 0, 0, MSGPACK_TYPE_NIL);
	}
	else if (type == json_type_boolean)
	{
		if (!json_object_get_boolean(input))
		{
			msgpack_object_data_set(out, NULL, 0, 0, MSGPACK_TYPE_FALSE);
		}
		else
		{
			msgpack_object_data_set(out, NULL, 0, 0, MSGPACK_TYPE_TRUE);
		}
	}
	else if (type == json_type_double)
	{
		double value = json_object_get_double(input);
		msgpack_object_data_set(out, &value, sizeof(value), 1, MSGPACK_TYPE_DOUBLE);
	}
	else
	{
		//json_type_int
		int64_t value = json_object_get_int64(input);
		msgpack_object_data_set(out, &value, get_int_size(value), 1, get_int_type(value));
	}

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

	if (type == json_type_object)
	{
		struct json_object_iterator it = json_object_iter_begin(input);
		struct json_object_iterator itEnd = json_object_iter_end(input);

		while (!json_object_iter_equal(&it, &itEnd))
		{
			//key
			json_object_to_msgpack_object(out, json_object_new_string(json_object_iter_peek_name(&it)));
			//value
			json_object_to_msgpack_object(out, json_object_iter_peek_value(&it));

			json_object_iter_next(&it);
		}
	}
	else if (type == json_type_array)
	{
		for (size_t i = 0; i < json_object_array_length(input); i++)
		{
			json_object_to_msgpack_object(out, json_object_array_get_idx(input, i));
		}
	}

	return out;
}

write_buff *json_to_msgpack_write_buff(json_object *input)
{
	if (!input)
	{
		//invalid json format
		return NULL;
	}

	return get_msgpack_write_buff(NULL, json_object_to_msgpack_object(NULL, input));
}

void *json_file_to_msgpack_binary(int fd, size_t *size)
{
	if((fd < 0) || (!size))
	{
		return NULL;
	}

	json_object *input = json_object_from_fd(fd);
	write_buff *buff = NULL;

	if (buff = json_to_msgpack_write_buff(input))
	{
		*size = buff->offset;
		return buff->data;
	}
	else
	{
		*size = 0;
		return NULL;
	}
}

void *json_string_to_msgpack_binary(char *string, size_t *size)
{
	if(!string || !size)
	{
		return NULL;
	}

	json_object *input = json_tokener_parse(string);
	write_buff *buff = NULL;
	
	if (buff = json_to_msgpack_write_buff(input))
	{
		*size = buff->offset;
		return buff->data;
	}
	else
	{
		*size = 0;
		return NULL;
	}
}