#ifndef __LIB_MENU_H__
#define __LIB_MENU_H__

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define DIRECTORY_ATTR 0
#define EXECUTION_ATTR 1

#define ECHO 1

typedef int (*cb_func)(int argc, const char* argv[], struct menu_command* menu_tbl);

typedef struct menu_command {
    char* name;
    char attr; // DIRECTORY_ATTR  0 EXECUTION_ATTR  1  LINUX_CMD_ATTR  2
    cb_func fp;
    char* usage;
    char* desc;
} menu_command_t;

int process_man_cmd(int argc, const char* argv[], menu_command_t* menu_tbl, char echo);
int lookup_cmd_tbl(int argc, const char* argv[], menu_command_t* menu_tbl, char echo);

#endif // __LIB_MENU_H__
