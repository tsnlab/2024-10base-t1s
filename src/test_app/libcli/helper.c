#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

/* str2long - safely convert string to int64_t
 *
 * @str: source string to convert from
 * @lng_ptr: pointer where to store result
 *
 * returns zero on success
 * returns (-1) if one of args is NULL, (-2) invalid input, (-3) for *flow
 */
int str2long(const char* str, int64_t* lng_ptr) {
    char* end_ptr = 0;
    if (!str || !lng_ptr) {
        return (-1);
    }

    *lng_ptr = 0;
    errno = 0;
    *lng_ptr = strtol(str, &end_ptr, 0);

    if (*end_ptr != '\0') {
        return (-2);
    }

    if (errno != 0) {
        return (-3);
    }

    return 0;
} /* str2long(...) */

/* str2int - safely convert string to int32_t
 *
 * @str: source string to convert from
 * @int_ptr: pointer where to store result
 *
 * returns zero on success
 * returns (-1) if one of args is NULL, (-2) invalid input, (-3) for *flow
 */
int str2int(const char* str, int32_t* int_ptr) {
    int rc = 0;
    int64_t arg_long = 0;
    if (!str || !int_ptr) {
        return (-1);
    }

    if ((rc = str2long(str, &arg_long)) != 0) {
        *int_ptr = 0;
        return rc;
    }

    if (arg_long < INT32_MIN || arg_long > INT32_MAX) {
        return (-3);
    }

    *int_ptr = (int32_t)arg_long;
    return 0;
} /* str2int(...) */
