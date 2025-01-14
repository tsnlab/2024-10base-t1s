# 10BASE-T1S Network Driver

## Overview

This project implements a network driver to support 10BASE-T1S Ethernet communication. The driver enables Host-to-Controller communication via SPI and allows Controllers to communicate over a 10BASE-T1S network. Raspberry Pi 4 is currently used as the host system, and LAN8651 chip as the node for development and testing.

Key features include:
- SPI communication with the LAN8651 chip.
- Ethernet Packet I/O.
- Timestamping during I/O operations.
- Tools for measuring latency and throughput.

## Current Progress
- **SPI Communication**: Initial communication with the 10BASE-T1S HAT board using the LAN8651 chip is done.
- **Implement Ethernet Packet I/O**: ARP I/O test is in progress.
- **Upcoming Milestones**:
  1. Add timestamping support for I/O operations.
  2. Organize and refine the codebase into a cohesive library or framework.
  3. Develop tools for latency and throughput measurement.

## Acknowledgments
- The LAN8651 datasheet and reference manual.
- IEEE 802.3cg standards documentation.

## License


## Contact
For questions or support, please open an issue or mail us to contact at tsnlab dot com.
