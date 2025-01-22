#include <ctype.h>
#include <string.h>

int is_hexdecimal(char* param) {
    int i, len;

    len = strlen(param);

    for (i = 0; i < len; i++) {
        if (!isxdigit(param[i])) {
            return -1;
        }
    }

    return 0;
}

int char_to_hexdecimal(char c) {
    if ((c >= '0') && (c <= '9')) {
        return (c - 48);
    } else if ((c >= 'A') && (c <= 'F')) {
        return (c - 55);
    } else if ((c >= 'a') && (c <= 'f')) {
        return (c - 87);
    } else {
        return -1;
    }
}

int fill_nbytes_string_2_hexadecimal(char* str, unsigned char* v, int n) {
    int i, id, len;
    int val = 0;

    for (i = 0; i < n; i++) {
        v[i] = 0x00;
    }

    len = strlen(str);
    if (len >= (n * 2)) {
        for (i = (len - (n * 2)), id = 0; i < len; i += 2, id++) {
            val = (char_to_hexdecimal(str[i]) << 4) + char_to_hexdecimal(str[i + 1]);
            v[id] = val & 0xFF;
        }
    } else {
        if (len % 2) {
            id = (n - 1) - (len / 2);
            val = char_to_hexdecimal(str[0]);
            v[id] = val & 0xFF;
            for (i = 1, id = (n - (len / 2)); i < len; i += 2, id++) {
                val = (char_to_hexdecimal(str[i]) << 4) + char_to_hexdecimal(str[i + 1]);
                v[id] = val & 0xFF;
            }
        } else {
            for (i = 0, id = (n - (len / 2)); i < len; i += 2, id++) {
                val = (char_to_hexdecimal(str[i]) << 4) + char_to_hexdecimal(str[i + 1]);
                v[id] = val & 0xFF;
            }
        }
    }

    return 0;
}
