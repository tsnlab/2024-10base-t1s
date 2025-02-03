#include <pthread.h>
#include <stdio.h>

#include "xbase-t1s.h"
#include "xbase_common.h"

int rx_thread_run = 1;
int tx_thread_run = 1;

static CircularQueue_t g_queue;
static CircularQueue_t* queue = NULL;

void initialize_queue(CircularQueue_t* p_queue) {
    queue = p_queue;

    p_queue->front = 0;
    p_queue->rear = -1;
    p_queue->count = 0;
    pthread_mutex_init(&p_queue->mutex, NULL);
}

static int isQueueEmpty() {
    return (queue->count == 0);
}

static int isQueueFull() {
    return (queue->count == NUMBER_OF_QUEUE);
}

int getQueueCount() {
    return queue->count;
}

static void xbuffer_enqueue(QueueElement element) {
    pthread_mutex_lock(&queue->mutex);

    if (isQueueFull()) {
        debug_printf("Queue is full. Cannot xbuffer_enqueue.\n");
        pthread_mutex_unlock(&queue->mutex);
        return;
    }

    queue->rear = (queue->rear + 1) % NUMBER_OF_QUEUE;
    queue->elements[queue->rear] = element;
    queue->count++;

    pthread_mutex_unlock(&queue->mutex);
}

static QueueElement xbuffer_dequeue() {
    pthread_mutex_lock(&queue->mutex);

    if (isQueueEmpty()) {
        debug_printf("Queue is empty. Cannot xbuffer_dequeue.\n");
        pthread_mutex_unlock(&queue->mutex);
        return EMPTY_ELEMENT;
    }

    QueueElement dequeuedElement = queue->elements[queue->front];
    queue->front = (queue->front + 1) % NUMBER_OF_QUEUE;
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return dequeuedElement;
}

static void receiver_as_client() {

    BUF_POINTER buffer;
    int bytes_rcv;

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {
        buffer = buffer_pool_alloc();
        if (buffer == NULL) {
            printf("FAILURE: Could not buffer_pool_alloc.\n");
            //            rx_stats.rxNoBuffer++;
            continue;
        }

        bytes_rcv = 0;
#if 0
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

void receiver_as_server() {

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
    case RUN_MODE_CLIENT:
        receiver_as_client();
        break;
    case RUN_MODE_SERVER:
        receiver_as_server();
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
