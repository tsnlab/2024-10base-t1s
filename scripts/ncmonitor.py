#!/usr/bin/env python3

import curses
import socket
from datetime import datetime, timedelta
from typing import NamedTuple

PORT = 1337


class Stat(NamedTuple):
    bps: int
    pps: int


# Listen on 1337/UDP and print received throughput by src ip
def main(stdscr):
    # Init screen
    curses.use_default_colors()
    curses.init_pair(1, -1, -1)
    stdscr.keypad(True)
    stdscr.clear()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Set non-blocking mode to avoid blocking recvfrom
    sock.setblocking(False)
    sock.bind(('0', PORT))
    stats: dict[str, Stat] = {}

    tick = datetime.now()
    while True:
        now = datetime.now()

        try:
            data, addr = sock.recvfrom(2000)
            ip = addr[0]
            if ip not in stats:
                stats[ip] = Stat(0, 0)
            datalen = 14 + 20 + 8 + len(data)  # Ethernet + IP + UDP + data
            stats[ip] = Stat(stats[ip].bps + datalen * 8, stats[ip].pps + 1)
        except BlockingIOError:
            pass
        except KeyboardInterrupt:
            break

        if now - tick >= timedelta(seconds=1):
            stdscr.clear()
            stdscr.addstr(0, 0, f'{now:%F %T}')
            for i, (ip, stat) in enumerate(sorted(stats.items())):
                stdscr.addstr(i + 1, 0, f'{ip}: {stat.bps:,} bps {stat.pps:,} pps')

            total_bps = sum(stat.bps for stat in stats.values())
            total_pps = sum(stat.pps for stat in stats.values())
            stdscr.addstr(len(stats) + 2, 0, f'Total: {total_bps:,} bps {total_pps:,} pps')
            stdscr.refresh()

            stats.clear()
            tick = now


if __name__ == '__main__':
    curses.wrapper(main)
