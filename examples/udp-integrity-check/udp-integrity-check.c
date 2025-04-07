#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFSIZE 2048
#define ETHHDRSIZE 14
#define IPHDRSIZE 20
#define UDPHDRSIZE 8
#define FCSSIZE 4

char* generate_random_message(int length) {
    char* message = malloc(length + 1);
    for (int i = 0; i < length; i++) {
        message[i] = rand() % 26 + 'a';
    }
    message[length] = 0;
    return message;
}

int main(int argc, char* argv[]) {
    int sockfd, n, count, success, fail;
    struct sockaddr_in servaddr, cliaddr;
    char *sendbuf, recvbuf[BUFSIZE];

    float pro_b = 0.0;
    int pro_p = 0, idx;
    char buff[100];

    /* check arguments */
    if (argc != 7) {
        printf("Usage: %s <destination_ip> <destination_port> <source_ip> <source_port> <message_length> <count>\n",
               argv[0]);
        exit(1);
    }

    /* parse message length and count */
    int msglen = atoi(argv[5]) - ETHHDRSIZE - IPHDRSIZE - UDPHDRSIZE - FCSSIZE;
    count = atoi(argv[6]);

    /* create socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        exit(1);
    }

    /* setup server address */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    /* setup client address */
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr(argv[3]);
    cliaddr.sin_port = htons(atoi(argv[4]));

    /* initialize random number generator */
    srand(time(NULL));

    /* initialize counters */
    success = 0;
    fail = 0;

    /* send and receive messages */
    for (int i = 0; i < count; i++) {
        sendbuf = generate_random_message(msglen);

        /* send message */
        if (sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            perror("sendto error");
            exit(1);
        }

        /* receive message */
        socklen_t len = sizeof(cliaddr);
        n = recvfrom(sockfd, recvbuf, BUFSIZE, 0, (struct sockaddr*)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom error");
            exit(1);
        }
        recvbuf[n] = 0;

        /* check message */
        if (strcmp(sendbuf, recvbuf) != 0) {
            printf("Fail: sent message [%s] does not match received message [%s]\n", sendbuf, recvbuf);
            fail++;
        } else {
            success++;
        }

        /* free message */
        free(sendbuf);

        pro_b = (i * 100.0) / (count * 1.0);
        pro_p = (i * 100) / (count);
        memset(buff, 0, 100);
        sprintf(buff, "Progress-Bar [");
        for (idx = 0; idx < (pro_p / 2); idx++)
            sprintf(buff, "%s%s", buff, "#");
        for (idx = (pro_p / 2); idx < 50; idx++)
            sprintf(buff, "%s%s", buff, " ");
        sprintf(buff, "%s%s (%.2f%%)", buff, "]", pro_b);
        printf("%s\r", buff);
        fflush(stdout);
    }

    memset(buff, 0, 100);
    sprintf(buff, "Progress-Bar [");
    for (idx = 0; idx < 50; idx++)
        sprintf(buff, "%s%s", buff, "#");
    sprintf(buff, "%s%s (%.2f%%)", buff, "]", 100.00);
    printf("%s\n", buff);

    /* print results */
    printf("Sent and received %d messages\n", count);
    printf("Success: %d\n", success);
    printf("Fail: %d\n", fail);

    /* close socket */
    close(sockfd);

    return 0;
}
