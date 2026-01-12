
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "fastcommon/hash.h"
#include "fastcommon/json_parser.h"

/*
 * Compatibility shim:
 * The system /lib/libshmcache.so on this image expects legacy fastcommon symbols
 * (simple_hash, hash_get_prime_capacity, decode_json_array, free_common_array),
 * but the installed libfastcommon exports the newer fc_* variants.
 *
 * We provide the legacy symbol names and forward to the fc_* implementations.
 */

#ifdef __cplusplus
extern "C" {
#endif

extern int fc_simple_hash(const void* key, const int key_len);
extern unsigned int* fc_hash_get_prime_capacity(const int capacity);

/* Newer fastcommon exposes JSON parsing via a context object. */
extern const fc_json_array_t* fc_decode_json_array(fc_json_context_t* context,
                                                  const string_t* input);

int simple_hash(const void* key, const int key_len) {
    return fc_simple_hash(key, key_len);
}

unsigned int* hash_get_prime_capacity(const int capacity) {
    return fc_hash_get_prime_capacity(capacity);
}

int decode_json_array(const string_t* input, void* array, char* error_info,
                      const int error_size) {
    if (input == NULL || array == NULL) {
        return EINVAL;
    }

    fc_json_context_t ctx;
    const bool decode_use_mpool = false;
    const int alloc_size_once = 0;
    const int init_buff_size = 0;

    int result = fc_init_json_context_ex(&ctx, decode_use_mpool, alloc_size_once,
                                         init_buff_size, error_info, error_size);
    if (result != 0) {
        return result;
    }

    const fc_json_array_t* parsed = fc_decode_json_array(&ctx, input);
    if (parsed == NULL) {
        result = fc_json_parser_get_error_no(&ctx);
        fc_destroy_json_context(&ctx);
        return result != 0 ? result : EINVAL;
    }

    const int count = parsed->count;
    size_t bytes_for_strings = 0;
    for (int i = 0; i < count; ++i) {
        bytes_for_strings += (size_t)parsed->elements[i].len + 1;
    }

    size_t bytes_for_elements = (size_t)count * sizeof(string_t);
    string_t* elements = (string_t*)malloc(bytes_for_elements + bytes_for_strings);
    if (elements == NULL) {
        fc_destroy_json_context(&ctx);
        return ENOMEM;
    }

    char* string_storage = (char*)(elements + count);
    for (int i = 0; i < count; ++i) {
        const int len = parsed->elements[i].len;
        elements[i].len = len;
        elements[i].str = string_storage;
        if (len > 0) {
            memcpy(string_storage, parsed->elements[i].str, (size_t)len);
            string_storage += len;
        }
        *string_storage++ = '\0';
    }

    /*
     * The legacy ABI expects the first fields of the array struct to match.
     * fc_json_array_t is compatible with common legacy typedefs here.
     */
    fc_json_array_t* out = (fc_json_array_t*)array;
    out->elements = elements;
    out->count = count;
    out->element_size = (int)sizeof(string_t);
    out->alloc = count;

    fc_destroy_json_context(&ctx);
    return 0;
}

void free_common_array(fc_common_array_t* array) {
    fc_free_common_array(array);
}

#ifdef __cplusplus
}
#endif
