#ifndef APP_XBASE_T1S_H
#define APP_XBASE_T1S_H

enum {
    RUN_MODE_CLIENT,
    RUN_MODE_SERVER,
    RUN_MODE_CNT,
};

#define DEFAULT_RUN_MODE RUN_MODE_CLIENT

typedef struct thread_arg {
    int mode;
    int node_id;
} rx_thread_arg_t, tx_thread_arg_t;

#endif /* APP_XBASE_T1S_H */
