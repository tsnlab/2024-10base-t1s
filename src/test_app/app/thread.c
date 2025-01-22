#include <stdio.h>

#include "xbase-t1s.h"

int rx_thread_run = 1;
int tx_thread_run = 1;

static void receiver_in_normal_mode() {

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {
#if 0
        buffer = buffer_pool_alloc();
        if (buffer == NULL) {
            debug_printf("FAILURE: Could not buffer_pool_alloc.\n");
            rx_stats.rxNoBuffer++;
            continue;
        }

        bytes_rcv = 0;
        if (xdma_api_read_to_buffer_with_fd(devname, fd, buffer, size, &bytes_rcv)) {
            if (buffer_pool_free(buffer)) {
                debug_printf("FAILURE: Could not buffer_pool_free.\n");
            }
            rx_stats.rxErrors++;
            continue;
        }
        if (bytes_rcv > MAX_BUFFER_LENGTH) {
            if (buffer_pool_free(buffer)) {
                debug_printf("FAILURE: Could not buffer_pool_free.\n");
            }
            rx_stats.rxErrors++;
            continue;
        }
        rx_stats.rxPackets++;
        rx_stats.rxBytes += bytes_rcv;

        xbuffer_enqueue((QueueElement)buffer);
#endif
    }
    printf("<<< %s\n", __func__);
}

void receiver_in_loopback_mode() {

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {

#if 0
        if (xdma_api_read_to_buffer_with_fd(devname, fd, buffer, size, &bytes_rcv)) {
            continue;
        }

        if (size != bytes_rcv) {
            debug_printf("FAILURE: size(%ld) and bytes_rcv(%ld) are different.\n", size, bytes_rcv);
            rx_stats.rxErrors++;
            continue;
        }

        if (memcmp((const void*)data, (const void*)buffer, size)) {
            debug_printf("FAILURE: data(%p) and buffer(%p) are different.\n", data, buffer);
            rx_stats.rxErrors++;
            continue;
        }

        rx_stats.rxPackets++;
        rx_stats.rxBytes += bytes_rcv;
#endif
    }
    printf("<<< %s\n", __func__);
}

void* receiver_thread(void* arg) {

    rx_thread_arg_t* p_arg = (rx_thread_arg_t*)arg;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

#if 0
    initialize_queue(&g_queue);
    initialize_statistics(&rx_stats);
#endif

    switch (p_arg->mode) {
    case RUN_MODE_NORMAL:
        receiver_in_normal_mode();
        break;
    case RUN_MODE_LOOPBACK:
        receiver_in_loopback_mode();
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, p_arg->mode);
        break;
    }

#if 0
    pthread_mutex_destroy(&g_queue.mutex);
#endif

    printf("<<< %s\n", __func__);

    return NULL;
}
