#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <lib_menu.h>

int process_main_run(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_main_read(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_main_write(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_main_config(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_main_test(int argc, const char* argv[], menu_command_t* menu_tbl);

int fn_config_node_argument(int argc, const char* argv[]);
int fn_config_mac_argument(int argc, const char* argv[]);
int fn_config_plca_argument(int argc, const char* argv[]);
int fn_config_reset_argument(int argc, const char* argv[]);

#if 0
int process_fpga_readCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_main_fpgaCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_aes_encryptCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_aes_readEccCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_aes_writeEccCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_aes_writeInstCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_aes_readInstCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_aes_actCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
int process_main_aesCmd(int argc, const char* argv[], menu_command_t* menu_tbl);
#endif
int command_parser(int argc, char** argv);

#endif // COMMAND_PARSER_H
