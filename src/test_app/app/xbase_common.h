#ifndef XBASE_COMMON_H
#define XBASE_COMMON_H

#define BUFFER_ALIGNMENT (0x800)
#define MAX_PACKET_LENGTH (0x800)
#define MAX_PACKET_BURST (1)
#define MAX_BUFFER_LENGTH (MAX_PACKET_LENGTH * MAX_PACKET_BURST)
#define NUMBER_OF_BUFFER (64)
#define NUMBER_OF_POOL_BUFFER (NUMBER_OF_BUFFER + 1)
#define NUMBER_OF_RESERVED_BUFFER (4)
#define ENGINE_NUMBER_OF_BUFFER (NUMBER_OF_BUFFER / 2)
#define PACKET_ADDRESS_MASK (~(MAX_PACKET_LENGTH - 1))

#define EMPTY_ELEMENT (NULL)

#define NUMBER_OF_QUEUE NUMBER_OF_BUFFER

typedef char* BUF_POINTER;

typedef struct buffer_stack {
    BUF_POINTER elements[NUMBER_OF_POOL_BUFFER];
    int top;
    pthread_mutex_t mutex;
} buffer_stack_t;

typedef struct reserved_buffer_stack {
    BUF_POINTER elements[NUMBER_OF_RESERVED_BUFFER];
    int top;
    pthread_mutex_t mutex;
} reserved_buffer_stack_t;

typedef char* QueueElement;
typedef struct circular_queue {
    QueueElement elements[NUMBER_OF_QUEUE];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
} CircularQueue_t;

/******************************************************************************
 *                                                                            *
 *                            Function Prototypes                             *
 *                                                                            *
 ******************************************************************************/

#endif // XBASE_COMMON_H
