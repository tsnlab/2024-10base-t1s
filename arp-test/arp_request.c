#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <unistd.h>

// Ethernet header
struct eth_hdr {
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short ethertype;
};

// ARP header
struct arp_hdr {
    unsigned short htype;
    unsigned short ptype;
    unsigned char hlen;
    unsigned char plen;
    unsigned short oper;
    unsigned char sender_mac[6];
    unsigned char sender_ip[4];
    unsigned char target_mac[6];
    unsigned char target_ip[4];
};

int main() {
    int sock;
    unsigned char buffer[42]; // Ethernet (14) + ARP (28)
    struct sockaddr_ll socket_address;
    struct eth_hdr *eth = (struct eth_hdr *) buffer;
    struct arp_hdr *arp = (struct arp_hdr *) (buffer + sizeof(struct eth_hdr));
    
    // Open raw socket
    sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Fill Ethernet header
    memset(eth->dest_mac, 0xFF, 6); // Broadcast
    memcpy(eth->src_mac, "\x20\x2b\x20\x61\x0f\x3f", 6); // Replace with your MAC
    eth->ethertype = htons(0x0806); // ARP Ethertype

    // Fill ARP header
    arp->htype = htons(1); // Ethernet
    arp->ptype = htons(0x0800); // IPv4
    arp->hlen = 6; // MAC size
    arp->plen = 4; // IP size
    arp->oper = htons(1); // ARP Request
    memcpy(arp->sender_mac, "\x20\x2b\x20\x61\x0f\x3f", 6); // Replace with your MAC
    inet_pton(AF_INET, "172.16.11.200", arp->sender_ip); // Replace with your IP
    memset(arp->target_mac, 0x00, 6); // Unknown
    inet_pton(AF_INET, "172.16.11.201", arp->target_ip); // Target IP

    // Fill sockaddr_ll
    memset(&socket_address, 0, sizeof(struct sockaddr_ll));
    socket_address.sll_ifindex = if_nametoindex("wlp2s0"); // Replace with your interface
    socket_address.sll_halen = ETH_ALEN;
    memset(socket_address.sll_addr, 0xFF, 6); // Broadcast

    // Send ARP request
    if (sendto(sock, buffer, 42, 0, (struct sockaddr *) &socket_address, sizeof(socket_address)) < 0) {
        perror("Send failed");
        return 1;
    }

    printf("ARP request sent\n");
    close(sock);
    return 0;
}