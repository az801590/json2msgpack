#include <stdio.h>
#include <stdint.h>
#include <json.h>

#include "json2msgpack.h"

enum INT_TYPE
{
	__FIXINT__,
	__N_FIXINT__,
	__UINT8__,
	__UINT16__,
	__UINT32__,
	__UINT64__,
	__INT8__,
	__INT16__,
	__INT32__,
	__INT64__
};

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

int writeMapFormat(size_t length, FILE *f)
{
	if (!f)
	{
		return 1;
	}

	uint8_t format = 0;
	if (length < (1 << 5))
	{
		format = 0x80 | length;
		fwrite(&format, sizeof(uint8_t), 1, f);
	}
	else if (length < (1 << 16))
	{
		uint16_t tmpLen = length;

		format = 0xde;
		toBigEndian(&tmpLen, sizeof(tmpLen));

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&tmpLen, sizeof(uint16_t), 1, f);
	}
	//4294967295 = (2^32)-1
	else if (length <= (4294967295))
	{
		uint32_t tmpLen = length;

		format = 0xdf;
		toBigEndian(&tmpLen, sizeof(uint32_t));

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&tmpLen, sizeof(uint32_t), 1, f);
	}
	/*
	//greater than (2^32)-1
	else
	{

	}
	*/

	return 0;
}

int writeArrayFormat(size_t length, FILE *f)
{
	if (!f)
	{
		return 1;
	}

	uint8_t format = 0;
	if (length < (1 << 4))
	{
		format = 0x90 | length;
		fwrite(&format, sizeof(uint8_t), 1, f);
	}
	else if (length < (1 << 16))
	{
		uint16_t tmpLen = length;

		format = 0xdc;
		toBigEndian(&tmpLen, sizeof(tmpLen));

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&tmpLen, sizeof(uint16_t), 1, f);
	}
	//4294967295 = (2^32)-1
	else if (length <= (4294967295))
	{
		uint32_t tmpLen = length;

		format = 0xdd;
		toBigEndian(&tmpLen, sizeof(uint32_t));

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&tmpLen, sizeof(uint32_t), 1, f);
	}
	/*
	//greater than (2^32)-1
	else
	{

	}
	*/

	return 0;
}

int writeStringFormat(size_t length, FILE *f)
{
	if (!f)
	{
		return 1;
	}

	uint8_t format = 0;
	if (length < (1 << 5))
	{
		format = 0xa0 | length;
		fwrite(&format, sizeof(uint8_t), 1, f);
	}
	else if (length < (1 << 8))
	{
		uint8_t tmpLen = length;

		format = 0xd9;

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&tmpLen, sizeof(uint8_t), 1, f);
	}
	else if (length < (1 << 16))
	{
		uint16_t tmpLen = length;

		format = 0xda;
		toBigEndian(&tmpLen, sizeof(tmpLen));

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&tmpLen, sizeof(uint16_t), 1, f);
	}
	//4294967295 = (2^32)-1
	else if (length <= (4294967295))
	{
		uint32_t tmpLen = length;

		format = 0xdb;
		toBigEndian(&tmpLen, sizeof(uint32_t));

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&tmpLen, sizeof(uint32_t), 1, f);
	}
	/*
	//greater than (2^32)-1
	else
	{

	}
	*/

	return 0;
}

enum INT_TYPE getIntType(const void *integer)
{
#ifdef __amd64__
	int64_t value = *(int64_t*)integer;
#elif defined __i386__
	int32_t value = *(int32_t*)integer;
#else
	int value = *(int*)integer;
#endif

	if(value >= 0)
	{
		if(value < (1 << 7))
		{
			return __FIXINT__;
		}
		else if(value < (1 << 8))
		{
			return __UINT8__;
		}
		else if(value < (1 << 16))
		{
			return __UINT16__;
		}
#ifdef __amd64__
		else if(value < 4294967296)
		{
			//4294967296 = 2^32
			return __UINT32__;
		}
		else
		{
			//uint64 or int64
			return __UINT64__;
		}
#elif defined __i386__
		else
		{
			//int32 or uint32
			return __UINT32__;
		}
#endif
	}
	else
	{
		if(value >= -(1 << 5))
		{
			return __N_FIXINT__;
		}
		else if(value >= -(1 << 7))
		{
			return __INT8__;
		}
		else if(value >= -(1 << 15))
		{
			return __INT16__;
		}
#ifdef __amd64__
		else if(value >= -2147483648)
		{
			//-2147483648 = -(2^31)
			return __INT32__;
		}
		else
		{
			return __INT64__;
		}
#elif defined __i386__
		else
		{
			return __INT32__;
		}
#endif
	}
}

void json2msgpack(json_object *input, FILE *f)
{
	json_type type = json_object_get_type(input);

	if (type == json_type_object)
	{
		writeMapFormat(json_object_object_length(input), f);
	}
	else if (type == json_type_array)
	{
		size_t arrLen = json_object_array_length(input);
		writeArrayFormat(arrLen, f);

		for (size_t i = 0; i < arrLen; i++)
		{
			json2msgpack(json_object_array_get_idx(input, i), f);
		}
		return;
	}
	else if (type == json_type_string)
	{
		size_t length = json_object_get_string_len(input);
		writeStringFormat(length, f);

		fwrite(json_object_get_string(input), sizeof(char), length, f);
		return;
	}
	else if (type == json_type_null)
	{
		uint8_t format = 0xc0;
		fwrite(&format, sizeof(uint8_t), 1, f);
		return;
	}
	else if (type == json_type_boolean)
	{
		uint8_t format = 0;
		if (!json_object_get_boolean(input))
		{
			//false
			format = 0xc2;
		}
		else
		{
			//true
			format = 0xc3;
		}
		fwrite(&format, sizeof(uint8_t), 1, f);
		return;
	}
	else if (type == json_type_double)
	{
		uint8_t format = 0xcb;
		double value = json_object_get_double(input);
		toBigEndian(&value, sizeof(value));

		fwrite(&format, sizeof(uint8_t), 1, f);
		fwrite(&value, sizeof(double), 1, f);
		return;
	}
	else
	{
		//json_type_int
		uint8_t format = 0;
#ifdef __amd64__
		int64_t value = json_object_get_int64(input);
		format = 0xd3;
#elif defined __i386__
		int32_t value = json_object_get_int(input);
		format = 0xd2;
#else
		int value = json_object_get_int(input);
		format = 0xd2;
#endif

		fwrite(&format, sizeof(format), 1, f);

		/*
		** always save as 32/64 signed integer
		*/
		toBigEndian(&value, sizeof(value));
		fwrite(&value, sizeof(value), 1, f);
		return;
	}

	struct json_object_iterator it = json_object_iter_begin(input);
	struct json_object_iterator itEnd = json_object_iter_end(input);

	while (!json_object_iter_equal(&it, &itEnd))
	{
		//key
		json2msgpack(json_object_new_string(json_object_iter_peek_name(&it)), f);
		//value
		json2msgpack(json_object_iter_peek_value(&it), f);

		json_object_iter_next(&it);
	}
}
