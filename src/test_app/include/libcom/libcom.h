#ifndef __LIBCOM_H__
#define __LIBCOM_H__

#include <stdint.h>

#define IS_NOT_FOLDER (-2)
#define NOT_EXIST_FOLDER (-1)
#define EXIST_FOLDER (0)

#define IS_NOT_FILE (-2)
#define NOT_EXIST_FILE (-1)
#define EXIST_FILE (0)

int is_hexdecimal(char* param);
int fill_nbytes_string_2_hexadecimal(char* str, unsigned char* v, int n);

uint64_t mac_to_int64(const char* mac_address);
uint32_t ipv4_to_int32(const char* ipv4_address);

void dump_buffer(unsigned char* buffer, int len);

void mac_address(unsigned char* mac, char* name);
void ipv4_address(unsigned char* ip, char* name);

#endif // __LIBCOM_H__
