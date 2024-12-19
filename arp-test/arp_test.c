#include <arp_test.h>

// ARP table
struct arp_entry g_arp_table[MAX_ENTRIES];
int g_arp_table_count = 0;

// Function to add an entry to the ARP table
void add_to_arp_table(unsigned char *ip, unsigned char *mac)
{
    int i;
    for(i = 0; i < g_arp_table_count; i++) {
        if(memcmp(g_arp_table[i].ip, ip, 4) == 0) {
            /* IP already exists, update MAC */
            memcpy(g_arp_table[i].mac, mac, 6);
            return;
        }
    }
    if (g_arp_table_count < MAX_ENTRIES) {
        memcpy(g_arp_table[g_arp_table_count].ip, ip, 4);
        memcpy(g_arp_table[g_arp_table_count].mac, mac, 6);
        g_arp_table_count++;
    }
    else {
        printf("ARP table is full\n");
    }
}

// Function to print the ARP table
void print_arp_table() {
    int i;

    printf("\nARP Table:\n");
    for (i = 0; i < g_arp_table_count; i++) {
        printf("IP: %d.%d.%d.%d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        g_arp_table[i].ip[0], g_arp_table[i].ip[1], g_arp_table[i].ip[2], g_arp_table[i].ip[3],
        g_arp_table[i].mac[0], g_arp_table[i].mac[1], g_arp_table[i].mac[2],
        g_arp_table[i].mac[3], g_arp_table[i].mac[4], g_arp_table[i].mac[5]);
    }
}


int main() {
    int sock;
    unsigned char buffer[PACKET_SIZE_ARP]; // Ethernet (14) + ARP (28)
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
    if (sendto(sock, buffer, PACKET_SIZE_ARP, 0, (struct sockaddr *) &socket_address, sizeof(socket_address)) < 0) {
        perror("Send failed");
        return 1;
    }

    printf("ARP request sent\n");

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
                        arp->sender_ip[0], arp->sender_ip[1], arp->sender_ip[2], arp->sender_ip[3]);
                add_to_arp_table(arp->sender_ip, arp->sender_mac);
                print_arp_table();
                break; // Exit after receiving one reply
            }
        }
    }

    close(sock);
    return 0;
}