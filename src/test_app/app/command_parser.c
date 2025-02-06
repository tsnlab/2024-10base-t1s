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

menu_command_t main_command_tbl[] = {
    {"run", EXECUTION_ATTR, process_main_run, "   run -r <role> -i <ip address>",
     "   Run xbase-t1s application as role\n"
     "            <role> default value: 0 (0: client, 1: server)\n"
     "      <ip address> default value: 192.168.10.11\n"},
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

pthread_mutex_t spi_mutex;

int watch_stop = 1;

extern int rx_thread_run;
extern int tx_thread_run;
extern int verbose;

#if 0
struct reginfo reg_open_alliance[] = {{"Identification Register", OA_ID},
                                      {"PHY Identification Register", OA_PHYID},
                                      {"Standard Capabilities", OA_STDCAP},
                                      {"Reset Control and Status Register", OA_RESET},
                                      {"Configuration 0 Register", OA_CONFIG0},
                                      {"Status 0 Register", OA_STATUS0},
                                      {"Status 1 Register", OA_STATUS1},
                                      {"Buffer Status Register", OA_BUFSTS},
                                      {"Interrupt Mask 0 Register", OA_IMASK0},
                                      {"Interrupt Mask 1 Register", OA_IMASK1},
                                      {"Transmit Timestamp Capture A (High)", TTSCAH},
                                      {"Transmit Timestamp Capture A (Low)", TTSCAL},
                                      {"Transmit Timestamp Capture B (High)", TTSCBH},
                                      {"Transmit Timestamp Capture B (Low)", TTSCBL},
                                      {"Transmit Timestamp Capture C (High)", TTSCCH},
                                      {"Transmit Timestamp Capture C (Low)", TTSCCL},
                                      {"Basic Control", BASIC_CONTROL},
                                      {"Basic Status", BASIC_STATUS},
                                      {"PHY Identifier 1 Register", PHY_ID1},
                                      {"PHY Identifier 2 Register", PHY_ID2},
                                      {"MMD Access Control Register", MMDCTRL},
                                      {"MMD Access Address/Data Register", MMDAD},
                                      {"", -1}};

struct reginfo reg_mac[] = {{"Network Control Register", MAC_NCR},
                            {"Network Configuration Register", MAC_NCFGR},
                            {"Hash Register Bottom", MAC_HRB},
                            {"Hash Register Top", MAC_HRT},
                            {"Specific Address 1 Bottom", MAC_SAB1},
                            {"Specific Address 1 Top", MAC_SAT1},
                            {"Specific Address 2 Bottom", MAC_SAB2},
                            {"Specific Address 2 Top", MAC_SAT2},
                            {"Specific Address 3 Bottom", MAC_SAB3},
                            {"Specific Address 3 Top", MAC_SAT3},
                            {"Specific Address 4 Bottom", MAC_SAB4},
                            {"Specific Address 4 Top", MAC_SAT4},
                            {"MAC Type ID Match 1", MAC_TIDM1},
                            {"MAC Type ID Match 2", MAC_TIDM2},
                            {"MAC Type ID Match 3", MAC_TIDM3},
                            {"MAC Type ID Match 4", MAC_TIDM4},
                            {"Specific Address Match 1 Bottom", MAC_SAMB1},
                            {"Specific Address Match 1 Top", MAC_SAMT1},
                            {"Timer Increment Sub-Nanoseconds", MAC_TISUBN},
                            {"Timestamp Seconds High", MAC_TSH},
                            {"Timestamp Seconds Low", MAC_TSL},
                            {"Timestamp Nanoseconds", MAC_TN},
                            {"TSU Timer Adjust", MAC_TA},
                            {"TSU Timer Increment", MAC_TI},
                            {"Buffer Manager Control", BMGR_CTL},
                            {"Statistics 0", STATS0},
                            {"Statistics 1", STATS1},
                            {"Statistics 2", STATS2},
                            {"Statistics 3", STATS3},
                            {"Statistics 4", STATS4},
                            {"Statistics 5", STATS5},
                            {"Statistics 6", STATS6},
                            {"Statistics 7", STATS7},
                            {"Statistics 8", STATS8},
                            {"Statistics 9", STATS9},
                            {"Statistics 10", STATS10},
                            {"Statistics 11", STATS11},
                            {"Statistics 12", STATS12},
                            {"", -1}};

struct reginfo reg_phy_pcs[] = {{"10BASE-T1S PCS Control", T1SPCSCTL},
                                {"10BASE-T1S PCS Status", T1SPCSSTS},
                                {"10BASE-T1S PCS Diagnostic 1", T1SPCSDIAG1},
                                {"10BASE-T1S PCS Diagnostic 2", T1SPCSDIAG2},
                                {"", -1}};

struct reginfo reg_phy_pma_pmd[] = {{"BASE-T1 PMA/PMD Extended Ability", T1PMAPMDEXTA},
                                    {"BASE-T1 PMA/PMD Control", T1PMAPMDCTL},
                                    {"10BASE-T1S PMA Control", T1SPMACTL},
                                    {"10BASE-T1S PMA Status", T1SPMASTS},
                                    {"10BASE-T1S Test Mode Control", T1STSTCTL},
                                    {"", -1}};

struct reginfo reg_phy_vendor_specific[] = {{"Control 1 Register", CTRL1},
                                            {"Status 1 Register", STS1},
                                            {"Status 2 Register", STS2},
                                            {"Status 3 Register", STS3},
                                            {"Interrupt Mask 1 Register", IMSK1},
                                            {"Interrupt Mask 2 Register", IMSK2},
                                            {"Counter Control Register", CTRCTRL},
                                            {"Transmit Opportunity Count (High)", TOCNTH},
                                            {"Transmit Opportunity Count (Low)", TOCNTL},
                                            {"BEACON Count (High)", BCNCNTH},
                                            {"BEACON Count (Low)", BCNCNTL},
                                            {"PLCA Multiple ID 0 Register", MULTID0},
                                            {"PLCA Multiple ID 1 Register", MULTID1},
                                            {"PLCA Multiple ID 2 Register", MULTID2},
                                            {"PLCA Multiple ID 3 Register", MULTID3},
                                            {"PLCA Reconciliation Sublayer Status", PRSSTS},
                                            {"Port Management 2", PRTMGMT2},
                                            {"Inactivity Watchdog Timeout (High)", IWDTOH},
                                            {"Inactivity Watchdog Timeout (Low)", IWDTOL},
                                            {"Transmit Match Control Register", TXMCTL},
                                            {"Transmit Match Pattern (High) Register", TXMPATH},
                                            {"Transmit Match Pattern (Low) Register", TXMPATL},
                                            {"Transmit Match Mask (High) Register", TXMMSKH},
                                            {"Transmit Match Mask (Low) Register", TXMMSKL},
                                            {"Transmit Match Location Register", TXMLOC},
                                            {"Transmit Matched Packet Delay Register", TXMDLY},
                                            {"Receive Match Control Register", RXMCTL},
                                            {"Receive Match Pattern (High) Register", RXMPATH},
                                            {"Receive Match Pattern (Low) Register", RXMPATL},
                                            {"Receive Match Mask (High) Register", RXMMSKH},
                                            {"Receive Match Mask (Low) Register", RXMMSKL},
                                            {"Receive Match Location Register", RXMLOC},
                                            {"Receive Matched Packet Delay Register", RXMDLY},
                                            {"Credit Based Shaper Stop Threshold (High) Register", CBSSPTHH},
                                            {"Credit Based Shaper Stop Threshold (Low) Register", CBSSPTHL},
                                            {"Credit Based Shaper Start Threshold (High) Register", CBSSTTHH},
                                            {"Credit Based Shaper Start Threshold (Low) Register", CBSSTTHL},
                                            {"Credit Based Shaper Slope Control Register", CBSSLPCTL},
                                            {"Credit Based Shaper Top Limit (High) Register", CBSTPLMTH},
                                            {"Credit Based Shaper Top Limit (Low) Register", CBSTPLMTL},
                                            {"Credit Based Shaper Bottom Limit (High) Register", CBSBTLMTH},
                                            {"Credit Based Shaper Bottom Limit (Low) Register", CBSBTLMTL},
                                            {"Credit Based Shaper Credit Counter (High) Register", CBSCRCTRH},
                                            {"Credit Based Shaper Credit Counter (Low) Register", CBSCRCTRL},
                                            {"Credit Based Shaper Control Register", CBSCTRL},
                                            {"PLCA Skip Control Register", PLCASKPCTL},
                                            {"PLCA Transmit Opportunity Skip Register", PLCATOSKP},
                                            {"Application Controlled Media Access Control Register", ACMACTL},
                                            {"Sleep Control 0 Register", SLPCTL0},
                                            {"Sleep Control 1 Register", SLPCTL1},
                                            {"Collision Detector Control 0 Register", CDCTL0},
                                            {"SQI Control Register", SQICTL},
                                            {"SQI Status 0 Register", SQISTS0},
                                            {"SQI Configuration 0 Register", SQICFG0},
                                            {"SQI Configuration 2 Register", SQICFG2},
                                            {"Analog Control 5", ANALOG5},
                                            {"OPEN Alliance Map ID and Version Register", MIDVER},
                                            {"PLCA Control 0 Register", PLCA_CTRL0},
                                            {"PLCA Control 1 Register", PLCA_CTRL1},
                                            {"PLCA Status Register", PLCA_STS},
                                            {"PLCA Transmit Opportunity Timer Register", PLCA_TOTMR},
                                            {"PLCA Burst Mode Register", PLCA_BURST},
                                            {"", -1}};

struct reginfo reg_miscellaneous[] = {{"Queue Transmit Configuration", QTXCFG},
                                      {"Queue Receive Configuration", QRXCFG},
                                      {"Pad Control", PADCTRL},
                                      {"Clock Output Control", CLKOCTL},
                                      {"Miscellaneous", MISC},
                                      {"Device Identification", DEVID},
                                      {"Bus Parity Control and Status", BUSPCS},
                                      {"Configuration Protection Control", CFGPRTCTL},
                                      {"SRAM Error Correction Code Control", ECCCTRL},
                                      {"SRAM Error Correction Code Status", ECCSTS},
                                      {"SRAM Error Correction Code Fault Injection Control", ECCFLTCTRL},
                                      {"Event Capture 0 Control", EC0CTRL},
                                      {"Event Capture 1 Control", EC1CTRL},
                                      {"Event Capture 2 Control", EC2CTRL},
                                      {"Event Capture 3 Control", EC3CTRL},
                                      {"Event Capture Read Status Register", ECRDSTS},
                                      {"Event Capture Total Counts Register", ECTOT},
                                      {"Event Capture Clock Seconds High Register", ECCLKSH},
                                      {"Event Capture Clock Seconds Low Register", ECCLKSL},
                                      {"Event Capture Clock Nanoseconds Register", ECCLKNS},
                                      {"Event Capture Read Timestamp Register 0", ECRDTS0},
                                      {"Event Capture Read Timestamp Register 1", ECRDTS1},
                                      {"Event Capture Read Timestamp Register 2", ECRDTS2},
                                      {"Event Capture Read Timestamp Register 3", ECRDTS3},
                                      {"Event Capture Read Timestamp Register 4", ECRDTS4},
                                      {"Event Capture Read Timestamp Register 5", ECRDTS5},
                                      {"Event Capture Read Timestamp Register 6", ECRDTS6},
                                      {"Event Capture Read Timestamp Register 7", ECRDTS7},
                                      {"Event Capture Read Timestamp Register 8", ECRDTS8},
                                      {"Event Capture Read Timestamp Register 9", ECRDTS9},
                                      {"Event Capture Read Timestamp Register 10", ECRDTS10},
                                      {"Event Capture Read Timestamp Register 11", ECRDTS11},
                                      {"Event Capture Read Timestamp Register 12", ECRDTS12},
                                      {"Event Capture Read Timestamp Register 13", ECRDTS13},
                                      {"Event Capture Read Timestamp Register 14", ECRDTS14},
                                      {"Event Capture Read Timestamp Register 15", ECRDTS15},
                                      {"Phase Adjuster Cycles Register", PACYC},
                                      {"Phase Adjuster Control Register", PACTRL},
                                      {"Event 0 Start Time Nanoseconds Register", EG0STNS},
                                      {"Event 0 Start Time Seconds Low Register", EG0STSECL},
                                      {"Event 0 Start Time Seconds High Register", EG0STSECH},
                                      {"Event 0 Pulse Width Register", EG0PW},
                                      {"Event 0 Idle Time Register", EG0IT},
                                      {"Event Generator 0 Control Register", EG0CTL},
                                      {"Event 1 Start Time Nanoseconds Register", EG1STNS},
                                      {"Event 1 Start Time Seconds Low Register", EG1STSECL},
                                      {"Event 1 Start Time Seconds High Register", EG1STSECH},
                                      {"Event 1 Pulse Width Register", EG1PW},
                                      {"Event 1 Idle Time Register", EG1IT},
                                      {"Event Generator 1 Control Register", EG1CTL},
                                      {"Event 2 Start Time Nanoseconds Register", EG2STNS},
                                      {"Event 2 Start Time Seconds Low Register", EG2STSECL},
                                      {"Event 2 Start Time Seconds High Register", EG2STSECH},
                                      {"Event 2 Pulse Width Register", EG2PW},
                                      {"Event 2 Idle Time Register", EG2IT},
                                      {"Event Generator 2 Control Register", EG2CTL},
                                      {"Event 3 Start Time Nanoseconds Register", EG3STNS},
                                      {"Event 3 Start Time Seconds Low Register", EG3STSECL},
                                      {"Event 3 Start Time Seconds High Register", EG3STSECH},
                                      {"Event 3 Pulse Width Register", EG3PW},
                                      {"Event 3 Idle Time Register", EG3IT},
                                      {"Event Generator 3 Control Register", EG3CTL},
                                      {"One Pulse-per-Second Control Register", PPSCTL},
                                      {"Synchronization Event Interrupt Enable Register", SEVINTEN},
                                      {"Synchronization Event Interrupt Disable Register", SEVINTDIS},
                                      {"Synchronization Event Interrupt Mask Status Register", SEVIM},
                                      {"Synchronization Event Status Register", SEVSTS},
                                      {"", -1}};
#endif

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

int do_run(int mode, uint32_t ipv4) {

    pthread_t tid1, tid2;
    rx_thread_arg_t rx_arg;
    tx_thread_arg_t tx_arg;
    uint64_t mac;
    int ret;

    printf(">>> %s(mode: %s, ipv4: %d.%d.%d.%d)\n", __func__, mode ? "SERVER" : "CLIENT", (ipv4 >> 24) & 0xFF,
           (ipv4 >> 16) & 0xFF, (ipv4 >> 8) & 0xFF, ipv4 & 0xFF);

#if 1
    switch (mode) {
    case RUN_MODE_CLIENT:
        return do_as_client_main();
    case RUN_MODE_SERVER:
        return do_as_server_main();
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, mode);
        break;
    }

    return 0;
#endif

    ret = api_spi_init();
    if (ret) {
        printf("[%s]Fail to initialize SPI(error code: %d)\n", __func__, ret);
        return ret;
    }

    mac = api_get_mac_address();
    if (mac == 0) {
        printf("[%s]MAC address(%ld) has not been set yet. Check your settings first !\n", __func__, mac);
        goto out_spi;
    }

    if (initialize_buffer_allocation()) {
        printf("[%s]Fail to initialize buffer allocation\n", __func__);
        goto out_spi;
    }

    pthread_mutex_init(&spi_mutex, NULL);

    register_signal_handler();

    memset(&rx_arg, 0, sizeof(rx_thread_arg_t));
    rx_arg.mode = mode;
    rx_arg.ipv4 = ipv4;
    rx_arg.mac = mac;
    pthread_create(&tid1, NULL, receiver_thread, (void*)&rx_arg);
    sleep(1);

    memset(&tx_arg, 0, sizeof(tx_thread_arg_t));
    tx_arg.mode = mode;
    tx_arg.ipv4 = ipv4;
    tx_arg.mac = mac;
    pthread_create(&tid2, NULL, sender_thread, (void*)&tx_arg);
    sleep(1);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    pthread_mutex_destroy(&spi_mutex);

out_buffer:
    buffer_release();
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

#define MAIN_RUN_OPTION_STRING "r:i:hv"
int process_main_run(int argc, const char* argv[], menu_command_t* menu_tbl) {
    int mode = DEFAULT_RUN_MODE;
    int argflag;
    uint32_t ipv4 = 0xc0a80a0b;

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

    return do_run(mode, ipv4);
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
