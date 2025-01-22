#ifndef __LIBCOM_H__
#define __LIBCOM_H__

#define IS_NOT_FOLDER (-2)
#define NOT_EXIST_FOLDER (-1)
#define EXIST_FOLDER (0)

#define IS_NOT_FILE (-2)
#define NOT_EXIST_FILE (-1)
#define EXIST_FILE (0)

int is_hexdecimal(char* param);
int fill_nbytes_string_2_hexadecimal(char* str, unsigned char* v, int n);
#endif // __LIBCOM_H__
