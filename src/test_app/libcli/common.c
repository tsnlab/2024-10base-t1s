#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
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

uint32_t ipv4_to_int32(const char* ipv4_address) {
    uint32_t result = 0;
    unsigned int values[4];

    if (sscanf(ipv4_address, "%d.%d.%d.%d", &values[0], &values[1], &values[2], &values[3]) == 4) {
        for (int i = 0; i < 4; i++) {
            result = (result << 8) | (values[i] & 0xFF);
        }
    }

    return result;
}

uint64_t mac_to_int64(const char* mac_address) {
    uint64_t result = 0;
    unsigned int values[6];

    if (sscanf(mac_address, "%x:%x:%x:%x:%x:%x", &values[5], &values[4], &values[3], &values[2], &values[1],
               &values[0]) == 6) {
        for (int i = 0; i < 6; i++) {
            result = (result << 8) | (values[i] & 0xFF);
        }
    }

    return result;
}

void dump_buffer(unsigned char* buffer, int len) {

    for (int idx = 0; idx < len; idx++) {
        if ((idx % 16) == 0) {
            printf("\n  ");
        }
        printf("0x%02x ", buffer[idx] & 0xFF);
    }
    printf("\n");
}
