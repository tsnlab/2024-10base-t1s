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

menu_command_t main_command_tbl[] = {
    {"run", EXECUTION_ATTR, process_main_run, "   run -r <role> -i <Node ID> -c <Node Count> -m <MAC address>",
     "   Run xbase-t1s application as role\n"
     "            <role> default value: 0 (0: client, 1: server)\n"
     "         <Node ID> default value: 1 (0: Coordinator, 1 ~ 0xFE: Follower)\n"
     "      <Node Count> default value: 8 (2 ~ 0xFE)\n"
     "     <MAC address> default value: d8:3a:95:30:23:<Node ID+1>\n"},
    {"read", EXECUTION_ATTR, process_main_read, "   show -m <Memory Map Selector> -a <Addressi(Hex)> -c <Count>\n",
     "   Read register value\n"
     "        <Memory Map Selector> default value: 0 (0x00: Open Alliance 10BASE-T1x MAC-PHY Standard Registers\n"
     "                                                0x01: MAC Registers\n"
     "                                                0x02: PHY PCS Registers\n"
     "                                                0x03: PHY PMA/PMD Registers\n"
     "                                                0x04: PHY Vendor Specific Registers\n"
     "                                                0x0A: Miscellaneous Register Descriptions\n"
     "                    <Address> default value: 0 (   -1: All registers in the Memory Map Selector group,\n"
     "                                                0 >=0: Start address)\n"
     "                      <Count> default value: 1\n"},
#if 0
    {"set", EXECUTION_ATTR, process_main_setCmd,
     "   set register [gen, rx, tx, h2c, c2h, irq, con, h2cs, c2hs, com, msix] <addr(Hex)> <data(Hex)>\n",
     "   set XDMA resource"},
    {"test", EXECUTION_ATTR, process_main_tx_timestamp_testCmd, "   test -s <size>",
     "   This option was created for the reproduction of the Tx timestamp error issue. (Debugging Purpose)\n"},
#endif
    {0, EXECUTION_ATTR, NULL, " ", " "}};

pthread_mutex_t spi_mutex;

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

int do_run(int mode, int node_id, int node_cnt, uint64_t mac) {

    pthread_t tid1, tid2;
    rx_thread_arg_t rx_arg;
    tx_thread_arg_t tx_arg;
    int ret;

    printf(">>> %s(mode: %s, node_id: %d, node_cnt: %d, MAC: %012lx\n", __func__, mode ? "SERVER" : "CLIENT", node_id,
           node_cnt, mac);

    ret = api_spi_init();
    if (ret) {
        printf("[%s]Fail to initialize SPI(error code: %d)\n", __func__, ret);
        return ret;
    }

    api_configure_plca_to_mac_phy(node_id, node_cnt, mac);

    if (initialize_buffer_allocation()) {
        printf("[%s]Fail to initialize buffer allocation\n", __func__);
        return -1;
    }

    pthread_mutex_init(&spi_mutex, NULL);

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
#endif

    pthread_mutex_destroy(&spi_mutex);

    buffer_release();

    return 0;
}

int do_read(int mms, int address, int count) {
}

uint64_t mac_to_int64(const char* mac_address) {
    uint64_t result = 0;
    unsigned int values[6];

    if (sscanf(mac_address, "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2], &values[3], &values[4],
               &values[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            result = (result << 8) | (values[i] & 0xFF);
        }
    }

    return result;
}

#define MAIN_RUN_OPTION_STRING "r:m:i:c:hv"
int process_main_run(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mode = DEFAULT_RUN_MODE;
    int node_id = 1;
    int node_cnt = 8;
    int argflag;
    uint64_t mac = 0xd83a95302300;

    while ((argflag = getopt(argc, (char**)argv, MAIN_RUN_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'r':
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
            if ((node_cnt < 2) || (node_cnt > 0xFE)) {
                printf("Node Count %d is out of range.", node_cnt);
                return -1;
            }
            break;

        case 'm':
            mac = mac_to_int64((const char*)optarg);
            if (mac == 0) {
                printf("Invalid parameter given or out of range for '-%c'.", (char)argflag);
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

    if (mac == 0xd83a95302300) {
        mac += (node_id + 1) & 0xFF;
    }

    return do_run(mode, node_id, node_cnt, mac);
}

#define MAIN_READ_OPTION_STRING "m:a:c:hv"
int process_main_read(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mms = 0;
    int address = 0;
    int count = 8;
    int argflag;
    uint64_t mac = 0xd83a95302300;

    while ((argflag = getopt(argc, (char**)argv, MAIN_READ_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'm':
            if (str2int(optarg, &mms) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.", (char)argflag);
                return -1;
            }
            switch (mms) {
            case 0x00: /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
            case 0x01: /* MAC Registers */
            case 0x02: /* PHY PCS Registers */
            case 0x03: /* PHY PMA/PMD Registers */
            case 0x04: /* PHY Vendor Specific Registers */
            case 0x0A: /* Miscellaneous Register Descriptions */
                break;
            default:
                printf("mms %d is out of range.", mms);
                return -1;
            }
            break;

        case 'a':
            if (str2int(optarg, &address) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.", (char)argflag);
                return -1;
            }
            if (address < 0) {
                printf("Address 0x%x is wrong.", address);
                return -1;
            }
            break;

        case 'c':
            if (str2int(optarg, &count) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.", (char)argflag);
                return -1;
            }
            if (count < 1) {
                printf("Count %d is out of range.", count);
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

    return do_read(mms, address, count);
}

int command_parser(int argc, char** argv) {

    return lookup_cmd_tbl(argc, (const char**)argv, main_command_tbl, ECHO);
}
