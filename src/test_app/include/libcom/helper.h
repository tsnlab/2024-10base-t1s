#ifndef _HELPER_H_
#define _HELPER_H_

#include <inttypes.h>
#include <stdbool.h>

#define BUF2STR_MAXIMUM_OUTPUT_SIZE (3 * 1024 + 1)

struct valstr {
    uint32_t val;
    const char* str;
};

int str2long(const char* str, int64_t* lng_ptr);
int str2int(const char* str, int32_t* int_ptr);
#endif // _HELPER_H_
