#ifndef APP_THREAD_H
#define APP_THREAD_H

#define MAX_COUNTERS 29

enum {
    COUNTERS_RXPACKETS,
    COUNTERS_RXBYTES,
    COUNTERS_RXERRORS,
    COUNTERS_RXNOBUF,
    COUNTERS_RXPPS,
    COUNTERS_RXBPS,
    COUNTERS_TXPACKETS,
    COUNTERS_TXBYTES,
    COUNTERS_TXFILTERED,
    COUNTERS_TXERRORS,
    COUNTERS_TXPPS,
    COUNTERS_TXBPS,

    COUNTERS_CNT,
};

#define SEC (1)
#define MIN (60 * SEC)
#define HOUR (60 * MIN)
#define DAY (24 * HOUR)

typedef struct _execTime {
    int day;
    int hour;
    int min;
    int sec;
} execTime_t;

typedef struct stats {
    unsigned long long rxPackets;
    unsigned long long rxBytes;
    unsigned long long rxErrors;
    unsigned long long rxNoBuffer; /* BD is not available */
    unsigned long long rxPps;
    unsigned long long rxBps;
    unsigned long long txPackets;
    unsigned long long txBytes;
    unsigned long long txFiltered;
    unsigned long long txErrors;
    unsigned long long txPps;
    unsigned long long txBps;
} stats_t;

void* receiver_thread(void* arg);
void* sender_thread(void* arg);
void* stats_thread(void* arg);

#endif /* APP_THREAD_H */
