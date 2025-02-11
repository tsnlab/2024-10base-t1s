#ifndef CREATE_ETHERNET_PACKET_H
#define CREATE_ETHERNET_PACKET_H

void create_arp_request_frame(unsigned char* frame, unsigned char* smac, const char* src_ip, const char* dst_ip);
void create_udp_packet(unsigned char* src_mac, unsigned char* dst_mac, const char* src_ip_str, const char* dst_ip_str,
                       uint16_t src_port, uint16_t dst_port, uint16_t frame_length, uint8_t* packet_buffer);

#endif /* CREATE_ETHERNET_PACKET_H */
