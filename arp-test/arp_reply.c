#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <unistd.h>

#define MAX_ENTRIES 100

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
    unsigned char sender_ip[6];
    unsigned char target_mac[6];
    unsigned char target_ip[6];
};

// ARP table entry
struct arp_entry {
    unsigned char ip[4];
    unsigned char mac[6];
};

// ARP table
struct arp_entry arp_table[MAX_ENTRIES];
int arp_table_count = 0;

// Function to add an entry to the ARP table
void add_to_arp_table(unsigned char *ip, unsigned char *mac)
{
    int i;
    for(i = 0; i < arp_table_count; i++) {
        if(memcmp(arp_table[i].ip, ip, 4) == 0) {
            /* IP already exists, update MAC */
            memcpy(arp_table[i].mac, mac, 6);
            return;
        }
    }
    if (arp_table_count < MAX_ENTRIES)
    {
        memcpy(arp_table[arp_table_count].ip, ip, 4);
        memcpy(arp_table[arp_table_count].mac, mac, 6);
        arp_table_count++;
    }
    else
    {
        printf("ARP table is full\n");
    }
}

// Function to print the ARP table
void print_arp_table() {
    int i;

    printf("\nARP Table:\n");
    for (i = 0; i < arp_table_count; i++) {
        printf("IP: %d.%d.%d.%d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        arp_table[i].ip[0], arp_table[i].ip[1], arp_table[i].ip[2], arp_table[i].ip[3],
        arp_table[i].mac[0], arp_table[i].mac[1], arp_table[i].mac[2],
        arp_table[i].mac[3], arp_table[i].mac[4], arp_table[i].mac[5]);
    }
}

int main() {
    int sock;
    unsigned char buffer[42]; // Ethernet (14) + ARP (28)
    struct eth_hdr *eth = (struct eth_hdr *)buffer;
    struct arp_hdr *arp = (struct arp_hdr *) (buffer + sizeof(struct eth_hdr));

    // Receive ARP Reply
    while (1) {
        int len = recv(sock, buffer, sizeof(buffer), 0);
        if (len < 0) {
            perror("Received failed");
            return 1;
        }

        eth = (struct eth_hdr *)buffer;
        if (ntohs(eth->ethertype) == 0x0806) { // Check if ARP
            arp = (struct arp_hdr *) (buffer + sizeof(struct eth_hdr));
            if (ntohs(arp->oper) == 2) { // ARP Reply
                printf("Received ARP reply from %d.%d.%d.%d\n",
                        arp->sender_ip[0], arp->sender_ip[0], arp->sender_ip[0], arp->sender_ip[0]);
                add_to_arp_table(arp->sender_ip, arp->sender_mac);
                print_arp_table();
                break; // Exit after receiving one reply
            }
        }
    }

    close(sock);
    return 0;
}
