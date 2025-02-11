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

#include "error_define.h"
#include "thread.h"
#include "xbase-t1s.h"
#include "xbase_common.h"

menu_command_t main_command_tbl[] = {
    {"run", EXECUTION_ATTR, process_main_run,
     "   run -r <role> -i <ip address> -t <target ip address> -s <statistics> -l <Packet length>",
     "   Run xbase-t1s application as role\n"
     "                   <role> default value: 0 (0: client, 1: server)\n"
     "             <ip address> default value: 192.168.10.11\n"
     "      <target ip address> default value: 192.168.10.21\n"
     "             <statistics> default value: 0 (0: deactivate, 1:activate)\n"
     "         <Packet lengths> default value: 1514 (60 ~ 1514)\n"},
    {"read", EXECUTION_ATTR, process_main_read, "   read -m <Memory Map Selector>\n",
     "   Read all register values in Memory Map Selector\n"
     "        <Memory Map Selector> default value: 0 ( 0: Open Alliance 10BASE-T1x MAC-PHY Standard Registers\n"
     "                                                 1: MAC Registers\n"
     "                                                 2: PHY PCS Registers\n"
     "                                                 3: PHY PMA/PMD Registers\n"
     "                                                 4: PHY Vendor Specific Registers\n"
     "                                                10: Miscellaneous Register Descriptions)\n"},
    {"write", EXECUTION_ATTR, process_main_write, "   write -m <Memory Map Selector> -a <Address> -d <Data>\n",
     "   Write data value to register at address in Memory Map Selector\n"
     "        <Memory Map Selector> default value: 0 ( 0: Open Alliance 10BASE-T1x MAC-PHY Standard Registers\n"
     "                                                 1: MAC Registers\n"
     "                                                 2: PHY PCS Registers\n"
     "                                                 3: PHY PMA/PMD Registers\n"
     "                                                 4: PHY Vendor Specific Registers\n"
     "                                                10: Miscellaneous Register Descriptions)\n"
     "                    <Address> default value: 0\n"
     "                       <Data> default value: 0\n"},
    {"config", EXECUTION_ATTR, process_main_config, "   config <element> <options parameters>\n",
     "   Configure elements with options parameters\n"
     "   You must configure node, mac, and plca in that order.\n"
     "        config node -i <Node ID> -c <Node Count>\n"
     "                 <Node ID> default value: 1 (0: Coordinator, 1 ~ 0xFE: Follower)\n"
     "              <Node Count> default value: 8 (2 ~ 0xFE)\n\n"
     "        config mac -m  <MAC address>\n"
     "             <MAC address> default value: d8:3a:95:30:23:42\n\n"
     "        config plca\n\n"},
    {"test", EXECUTION_ATTR, process_main_test,
     "   run -r <role> -m <dst. MAC address> -i <ip address> -t <target ip address> -s <statistics> -l <Packet length>",
     "   Run xbase-t1s application as role\n"
     "                   <role> default value: 0 (0: reciever, 1: transmitter, 2: tranceiver)\n"
     "       <dst. MAC address> default value: d8:3a:95:30:23:42\n"
     "             <ip address> default value: 192.168.10.11\n"
     "      <target ip address> default value: 192.168.10.21\n"
     "             <statistics> default value: 0 (0: deactivate, 1:activate)\n"
     "         <Packet lengths> default value: 1514 (60 ~ 1514)\n"},
#if 0
    {"set", EXECUTION_ATTR, process_main_setCmd,
     "   set register [gen, rx, tx, h2c, c2h, irq, con, h2cs, c2hs, com, msix] <addr(Hex)> <data(Hex)>\n",
     "   set XDMA resource"},
    {"test", EXECUTION_ATTR, process_main_tx_timestamp_testCmd, "   test -s <size>",
     "   This option was created for the reproduction of the Tx timestamp error issue. (Debugging Purpose)\n"},
#endif
    {0, EXECUTION_ATTR, NULL, " ", " "}};

argument_list_t config_argument_tbl[] = {{"node", fn_config_node_argument},
                                         {"mac", fn_config_mac_argument},
                                         {"plca", fn_config_plca_argument},
                                         {0, NULL}};

int watch_stop = 1;

extern int rx_thread_run;
extern int tx_thread_run;
extern int stats_thread_run;
extern int verbose;
extern unsigned char my_mac[HW_ADDR_LEN];
extern unsigned char my_ipv4[IP_ADDR_LEN];
extern unsigned char dst_ipv4[IP_ADDR_LEN];

void signal_handler(int sig) {

    printf("\n10Base-T1S is exiting, cause (%d)!!\n", sig);
    tx_thread_run = 0;
    sleep(1);
    rx_thread_run = 0;
    sleep(1);
    stats_thread_run = 0;
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

uint32_t ipv4 = 0xc0a80a0b;

int drv_client_main();
int drv_server_main();

int do_as_server_main() {

    drv_init_server();

    while (rx_thread_run) {
        drv_server_main();
        sleep(1);
    }

    return drv_spi_cleanup();
}

int do_as_client_main() {

    drv_init_client();

    while (rx_thread_run) {
        drv_client_main();
        sleep(1);
    }

    return drv_spi_cleanup();
}

int fill_my_mac_address() {
    int i;
    uint64_t mac;

    mac = api_get_mac_address();
    if (mac == 0) {
        printf("[%s]MAC address(%ld) has not been set yet. Check your settings first !\n", __func__, mac);
        return -1;
    }

    for (i = 0; i < HW_ADDR_LEN; i++) {
        my_mac[i] = (unsigned char)((mac >> (i * 8)) & 0xff);
    }

    printf("MY MAC: \n    ");
    for (i = 0; i < HW_ADDR_LEN - 1; i++) {
        printf("%02x:", my_mac[i]);
    }
    printf("%02x\n", my_mac[i]);

    return 0;
}

void fill_ipv4_address(unsigned char* b_ipv4, uint32_t a_ipv4, char* name) {
    int i;

    for (i = 0; i < IP_ADDR_LEN; i++) {
        b_ipv4[IP_ADDR_LEN - 1 - i] = (unsigned char)((a_ipv4 >> (i * 8)) & 0xff);
    }
    printf("%s: \n    ", name);
    for (i = 0; i < IP_ADDR_LEN - 1; i++) {
        printf("%d.", b_ipv4[i]);
    }
    printf("%d\n", b_ipv4[i]);
}

int init_driver(int mode) {

    switch (mode) {
    case RUN_MODE_CLIENT:
    case TEST_MODE_TRANCEIVER:
        drv_init_client();
        break;
    case RUN_MODE_SERVER:
        drv_init_server();
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, mode);
        return -1;
    }
    return 0;
}

int do_run(int mode, uint32_t ipv4, uint32_t t_ipv4, int sts_flag, int pkt_length) {

    pthread_t tid1, tid2, tid3;
    rx_thread_arg_t rx_arg;
    tx_thread_arg_t tx_arg;
    stats_thread_arg_t st_arg;

    printf(">>> %s(mode: %s)\n", __func__, mode ? "SERVER" : "CLIENT");

    if (init_driver(mode)) {
        return -1;
    }

    if (fill_my_mac_address()) {
        goto out_spi;
    }

    fill_ipv4_address((unsigned char*)my_ipv4, ipv4, "My IPv4");
    fill_ipv4_address((unsigned char*)dst_ipv4, t_ipv4, "Dst. IPv4");

    register_signal_handler();

    memset(&rx_arg, 0, sizeof(rx_thread_arg_t));
    rx_arg.mode = mode;
    rx_arg.sts_flag = sts_flag;
    rx_arg.pkt_length = pkt_length;
    pthread_create(&tid1, NULL, receiver_thread, (void*)&rx_arg);
    sleep(1);

    memset(&tx_arg, 0, sizeof(tx_thread_arg_t));
    tx_arg.mode = mode;
    tx_arg.sts_flag = sts_flag;
    tx_arg.pkt_length = pkt_length;
    pthread_create(&tid2, NULL, sender_thread, (void*)&tx_arg);
    sleep(1);

    if (sts_flag) {
        memset(&st_arg, 0, sizeof(stats_thread_arg_t));
        st_arg.mode = mode;
        st_arg.sts_flag = sts_flag;
        pthread_create(&tid3, NULL, stats_thread, (void*)&st_arg);
    }
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    if (sts_flag) {
        pthread_join(tid3, NULL);
    }

out_spi:
    api_spi_cleanup();

    return 0;
}

int do_test(int mode, uint32_t ipv4, uint32_t t_ipv4, int sts_flag, int pkt_length, uint64_t dmac) {

    pthread_t tid1, tid2, tid3;
    rx_thread_arg_t rx_arg;
    tx_thread_arg_t tx_arg;
    stats_thread_arg_t st_arg;

    printf(">>> %s(mode: %s)\n", __func__, mode ? "SERVER" : "CLIENT");

    if (init_driver(mode)) {
        return -1;
    }

    if (fill_my_mac_address()) {
        goto out_spi;
    }

    fill_ipv4_address((unsigned char*)my_ipv4, ipv4, "My IPv4");
    fill_ipv4_address((unsigned char*)dst_ipv4, t_ipv4, "Dst. IPv4");

    register_signal_handler();

    /* Reciever or Tranceiver */
    if (mode == TEST_MODE_RECEIVER) {
        memset(&rx_arg, 0, sizeof(rx_thread_arg_t));
        rx_arg.mode = mode;
        rx_arg.sts_flag = sts_flag;
        rx_arg.pkt_length = pkt_length;
        rx_arg.dmac = dmac;
        pthread_create(&tid1, NULL, test_receiver_thread, (void*)&rx_arg);
        sleep(1);
    }

    /* Transmitter or Tranceiver */
    if ((mode == TEST_MODE_TRANSMITTER) || (mode == TEST_MODE_TRANCEIVER)) {
        memset(&tx_arg, 0, sizeof(tx_thread_arg_t));
        tx_arg.mode = mode;
        tx_arg.sts_flag = sts_flag;
        tx_arg.pkt_length = pkt_length;
        tx_arg.dmac = dmac;
        pthread_create(&tid2, NULL, transmitter_thread, (void*)&tx_arg);
        sleep(1);
    }

    if (sts_flag) {
        memset(&st_arg, 0, sizeof(stats_thread_arg_t));
        st_arg.mode = mode;
        st_arg.sts_flag = sts_flag;
        pthread_create(&tid3, NULL, stats_thread, (void*)&st_arg);
    }

    if (mode == TEST_MODE_RECEIVER) {
        pthread_join(tid1, NULL);
    }

    if ((mode == TEST_MODE_TRANSMITTER) || (mode == TEST_MODE_TRANCEIVER)) {
        pthread_join(tid2, NULL);
    }
    if (sts_flag) {
        pthread_join(tid3, NULL);
    }

out_spi:
    api_spi_cleanup();

    return 0;
}

int do_read(int mms) {
    int spi_ret;

    spi_ret = api_spi_init();
    if (spi_ret != 0) {
        printf("spi_init failed; the error code is %d\n", spi_ret);
        return -1;
    }
    return api_read_register_in_mms(mms);
}

int do_write(int mms, int addr, int data) {
    int spi_ret;

    spi_ret = api_spi_init();
    if (spi_ret != 0) {
        printf("spi_init failed; the error code is %d\n", spi_ret);
        return -1;
    }
    return api_write_register_in_mms(mms, addr, data);
}

int do_config_plca() {
    int spi_ret;

    spi_ret = api_spi_init();
    if (spi_ret != 0) {
        printf("spi_init failed; the error code is %d\n", spi_ret);
        return -1;
    }
    return api_configure_plca_to_mac_phy();
}

int do_config_mac(uint64_t mac) {
    int spi_ret;

    spi_ret = api_spi_init();
    if (spi_ret != 0) {
        printf("spi_init failed; the error code is %d\n", spi_ret);
        return -1;
    }
    return api_config_mac_address(mac);
}

int do_config_node(int node_id, int node_cnt) {
    int spi_ret;

    spi_ret = api_spi_init();
    if (spi_ret != 0) {
        printf("spi_init failed; the error code is %d\n", spi_ret);
        return -1;
    }
    return api_config_node(node_id, node_cnt);
}

#define MAIN_RUN_OPTION_STRING "r:i:t:s:l:hv"
int process_main_run(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mode = DEFAULT_RUN_MODE;
    int sts_flag = 0;
    int pkt_length = 1514;
    int argflag;
    uint32_t ipv4 = 0xc0a80a0b;
    uint32_t t_ipv4 = 0xc0a80a15;

    while ((argflag = getopt(argc, (char**)argv, MAIN_RUN_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'r':
            if (str2int(optarg, &mode) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((mode < 0) || (mode >= RUN_MODE_CNT)) {
                printf("mode %d is out of range.\n", mode);
                return -1;
            }
            break;

        case 'i':
            ipv4 = ipv4_to_int32((const char*)optarg);
            if (ipv4 == 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            break;

        case 't':
            t_ipv4 = ipv4_to_int32((const char*)optarg);
            if (t_ipv4 == 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            break;

        case 's':
            if (str2int(optarg, &sts_flag) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((sts_flag < 0) || (sts_flag > 1)) {
                printf("statistics %d is out of range.\n", sts_flag);
                return -1;
            }
            break;

        case 'l':
            if (str2int(optarg, &pkt_length) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((pkt_length < 60) || (pkt_length > 1514)) {
                printf("Packet lengths %d is out of range.\n", pkt_length);
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

    return do_run(mode, ipv4, t_ipv4, sts_flag, pkt_length);
}

#define MAIN_TEST_OPTION_STRING "r:m:i:t:s:l:hv"
int process_main_test(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mode = DEFAULT_RUN_MODE;
    int sts_flag = 0;
    int pkt_length = 1514;
    int argflag;
    uint32_t ipv4 = 0xc0a80a0b;
    uint32_t t_ipv4 = 0xc0a80a15;
    uint64_t mac = 0xd83a95302342;

    while ((argflag = getopt(argc, (char**)argv, MAIN_TEST_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'r':
            if (str2int(optarg, &mode) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((mode < 0) || (mode >= TEST_MODE_CNT)) {
                printf("mode %d is out of range.\n", mode);
                return -1;
            }
            break;

        case 'i':
            ipv4 = ipv4_to_int32((const char*)optarg);
            if (ipv4 == 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            break;

        case 't':
            t_ipv4 = ipv4_to_int32((const char*)optarg);
            if (t_ipv4 == 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            break;

        case 's':
            if (str2int(optarg, &sts_flag) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((sts_flag < 0) || (sts_flag > 1)) {
                printf("statistics %d is out of range.\n", sts_flag);
                return -1;
            }
            break;

        case 'l':
            if (str2int(optarg, &pkt_length) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((pkt_length < 60) || (pkt_length > 1514)) {
                printf("Packet lengths %d is out of range.\n", pkt_length);
                return -1;
            }
            break;

        case 'm':
            mac = mac_to_int64((const char*)optarg);
            if (mac == 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
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

    return do_test(mode, ipv4, t_ipv4, sts_flag, pkt_length, mac);
}

#define MAIN_READ_OPTION_STRING "m:hv"
int process_main_read(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mms = 0;
    int argflag;

    while ((argflag = getopt(argc, (char**)argv, MAIN_READ_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'm':
            if (str2int(optarg, &mms) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
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
                printf("mms %d is out of range.\n", mms);
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

    return do_read(mms);
}

#define MAIN_WRITE_OPTION_STRING "m:a:d:hv"
int process_main_write(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mms = 0;
    int addr = 0;
    int64_t data = 0;
    int argflag;

    while ((argflag = getopt(argc, (char**)argv, MAIN_WRITE_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'm':
            if (str2int(optarg, &mms) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
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
                printf("mms %d is out of range.\n", mms);
                return -1;
            }
            break;

        case 'a':
            if (str2int(optarg, &addr) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((addr < 0) || (addr > 0xFFFF)) {
                printf("Address 0x%x is out of range.\n", addr);
                return -1;
            }
            break;

        case 'd':
            if (str2long(optarg, &data) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((data < 0) || (data > 0xFFFFFFFF)) {
                printf("Data 0x%lx is out of range.\n", data);
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

    return do_write(mms, addr, (uint32_t)(data & 0xFFFFFFFF));
}

#define CONFIG_PLCA_OPTION_STRING "hv"
int fn_config_plca_argument(int argc, const char* argv[]) {
    int argflag;

    while ((argflag = getopt(argc, (char**)argv, CONFIG_PLCA_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'v':
            log_level_set(++verbose);
            if (verbose == 2) {
                /* add version info to debug output */
                lprintf(LOG_DEBUG, "%s\n", VERSION_STRING);
            }
            break;
        }
    }

    return do_config_plca();
}

#define CONFIG_MAC_OPTION_STRING "m:hv"
int fn_config_mac_argument(int argc, const char* argv[]) {
    uint64_t mac = 0xd83a95302342;
    int argflag;

    while ((argflag = getopt(argc, (char**)argv, CONFIG_MAC_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'm':
            mac = mac_to_int64((const char*)optarg);
            if (mac == 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
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
        }
    }

    return do_config_mac(mac);
}

#define CONFIG_NODE_OPTION_STRING "i:c:hv"
int fn_config_node_argument(int argc, const char* argv[]) {
    int node_id = 1;
    int node_cnt = 8;
    int argflag;

    while ((argflag = getopt(argc, (char**)argv, CONFIG_NODE_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'i':
            if (str2int(optarg, &node_id) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((node_id < 0) || (node_id > 0xFE)) {
                printf("Node ID %d is out of range.\n", node_id);
                return -1;
            }
            break;

        case 'c':
            if (str2int(optarg, &node_cnt) != 0) {
                printf("Invalid parameter given or out of range for '-%c'.\n", (char)argflag);
                return -1;
            }
            if ((node_cnt < 2) || (node_cnt > 0xFE)) {
                printf("Node Count %d is out of range.\n", node_cnt);
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
        }
    }

    return do_config_node(node_id, node_cnt);
}

int process_main_config(int argc, const char* argv[], menu_command_t* menu_tbl) {

    if ((argc <= 1) || (!strcmp(argv[1], "-h"))) {
        print_argument_warning_message(argc, argv, menu_tbl, NO_ECHO);
        return ERR_PARAMETER_MISSED;
    }

    argv++, argc--;
    for (int index = 0; config_argument_tbl[index].name; index++)
        if (!strcmp(argv[0], config_argument_tbl[index].name)) {
            config_argument_tbl[index].fp(argc, argv);
            return 0;
        }

    return ERR_INVALID_PARAMETER;
}

int command_parser(int argc, char** argv) {

    return lookup_cmd_tbl(argc, (const char**)argv, main_command_tbl, ECHO);
}
