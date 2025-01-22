#include "command_parser.h"

#include <api_device_driver.h>
#include <helper.h>
#include <lib_menu.h>
#include <libcom.h>
#include <log.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <version.h>

#include "thread.h"
#include "xbase-t1s.h"

menu_command_t main_command_tbl[] = {{"run", EXECUTION_ATTR, process_main_run,
                                      "   run -m <mode> -i <Node ID> -c <Node Count>",
                                      "   Run xbase-t1s application in mode\n"
                                      "            <mode> default value: 0 (0: normal, 1: loopback)\n"
                                      "         <Node ID> default value: 1 (0: Coordinator, 1 ~ 0xFE: Follower)\n"
                                      "      <Node Count> default value: 8 (2 ~ 0xFE)\n"},
#if 0
    {"show", EXECUTION_ATTR, process_main_showCmd,
     "   show register [gen, rx, tx, h2c, c2h, irq, con, h2cs, c2hs, com, msix]\n", "   Show XDMA resource"},
    {"set", EXECUTION_ATTR, process_main_setCmd,
     "   set register [gen, rx, tx, h2c, c2h, irq, con, h2cs, c2hs, com, msix] <addr(Hex)> <data(Hex)>\n",
     "   set XDMA resource"},
    {"test", EXECUTION_ATTR, process_main_tx_timestamp_testCmd, "   test -s <size>",
     "   This option was created for the reproduction of the Tx timestamp error issue. (Debugging Purpose)\n"},
#endif
                                     {0, EXECUTION_ATTR, NULL, " ", " "}};

int watch_stop = 1;

extern int rx_thread_run;
extern int tx_thread_run;
extern int verbose;

#if 0
int process_int_arg(const char* optarg, int* value, const char* err_msg) {
    if (str2int(optarg, value) != 0) {
        lprintf(LOG_ERR, "Invalid parameter given or out of range for '%s'.", err_msg);
        return -1;
    }
    if (*value < 0) {
        lprintf(LOG_ERR, "%s %d is out of range.", err_msg, *value);
        return -1;
    }
    return 0;
}

void process_verbose_option() {
    log_level_set(++verbose);
    if (verbose == 2) {
        /* add version info to debug output */
        lprintf(LOG_DEBUG, "%s\n", VERSION_STRING);
    }
}

int process_hex_arg(const char* optarg, unsigned char* output_buf, int length) {
    char data_buf[256];
    char* ptr_buf;

    memset(data_buf, 0, sizeof(data_buf));
    strcpy(data_buf, optarg);
    ptr_buf = (strncmp(data_buf, "0x", 2) && strncmp(data_buf, "0X", 2)) ? data_buf : &data_buf[2];

    if (is_hexdecimal(ptr_buf)) {
        lprintf(LOG_ERR, "data-value(%s) is invalid.", data_buf);
        return -1;
    }
    fill_nbytes_string_2_hexadecimal(ptr_buf, output_buf, length);
    return 0;
}
#endif

void signal_handler(int sig) {

    printf("\nXDMA-APP is exiting, cause (%d)!!\n", sig);
    tx_thread_run = 0;
    sleep(1);
    rx_thread_run = 0;
    sleep(1);
    exit(0);
}

void stop_signal_handler() {

    if (watch_stop) {
        signal_handler(2);
    } else {
        watch_stop = 1;
    }
}

void register_signal_handler() {

    signal(SIGINT, stop_signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGABRT, signal_handler);
}

int do_run(int mode, int node_id, int node_cnt) {

    pthread_t tid1, tid2;
    rx_thread_arg_t rx_arg;
    tx_thread_arg_t tx_arg;

    if (api_spi_init()) {
        return -1;
    }

    register_signal_handler();

    memset(&rx_arg, 0, sizeof(rx_thread_arg_t));
    rx_arg.mode = mode;
    rx_arg.node_id = node_id;
    pthread_create(&tid1, NULL, receiver_thread, (void*)&rx_arg);
    sleep(1);

#if 0
    memset(&tx_arg, 0, sizeof(tx_thread_arg_t));
    tx_arg.mode = mode;
    pthread_create(&tid2, NULL, sender_thread, (void*)&tx_arg);
    sleep(1);
#endif

    pthread_join(tid1, NULL);
#if 0
    pthread_join(tid2, NULL);

    buffer_release();
#endif

    return 0;
}

#define MAIN_RUN_OPTION_STRING "m:i:c:hv"
int process_main_run(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mode = DEFAULT_RUN_MODE;
    int node_id = 1;
    int node_cnt = 8;
    int argflag;

    while ((argflag = getopt(argc, (char**)argv, MAIN_RUN_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'm':
            if (str2int(optarg, &mode) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.", (char)argflag);
                return -1;
            }
            if ((mode < 0) || (mode >= RUN_MODE_CNT)) {
                printf("mode %d is out of range.", mode);
                return -1;
            }
            break;

        case 'i':
            if (str2int(optarg, &node_id) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.", (char)argflag);
                return -1;
            }
            if ((node_id < 0) || (node_id > 0xFE)) {
                printf("Node ID %d is out of range.", node_id);
                return -1;
            }
            break;

        case 'c':
            if (str2int(optarg, &node_cnt) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.", (char)argflag);
                return -1;
            }
            if ((node_cnt < 0) || (node_cnt > 0xFE)) {
                printf("Node Count %d is out of range.", node_cnt);
                return -1;
            }
            break;

        case 'v':
            log_level_set(++verbose);
            if (verbose == 2) {
                /* add version info to debug output */
                lprintf(LOG_DEBUG, "%s\n", VERSION_STRING);
            }
            break;

        case 'h':
            process_man_cmd(argc, argv, menu_tbl, ECHO);
            return 0;
        }
    }

    return do_run(mode, node_id, node_cnt);
}

int command_parser(int argc, char** argv) {

    return lookup_cmd_tbl(argc, (const char**)argv, main_command_tbl, ECHO);
}
