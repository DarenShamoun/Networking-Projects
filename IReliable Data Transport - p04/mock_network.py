"""
Module: mock_network

A program that simulates delay and loss over a network connection.
"""

import argparse
import logging
import socket
import random
from struct import unpack
from time import sleep

# configure the logger
FORMAT = '%(asctime)-15s %(levelname)-10s %(message)s'
logging.basicConfig(format=FORMAT)
LOGGER = logging.getLogger()

BUFFER_SIZE = 2 ** 12  # 4096. Keep buffer size as power of 2.

def simulate_lossy_send(proxy_socket, data, address, loss_rate, median_delay):
    if random.random() >= loss_rate:
        actual_delay = int(random.normalvariate(median_delay, 0.1*median_delay))
        if actual_delay < 0:
            actual_delay = 0

        LOGGER.debug(f'Forwarding segment with delay of {actual_delay}ms')
        sleep(actual_delay / 1000)
        proxy_socket.sendto(data, address)
    else:
        LOGGER.debug('Dropping packet')

def udp_proxy(src: str, dst: str, loss: float, median_delay: int):
    """Run UDP proxy.

    Arguments:
    src -- Source IP address and port string. I.e.: '127.0.0.1:8000'
    dst -- Destination IP address and port. I.e.: '127.0.0.1:8888'
    """
    LOGGER.info('Starting UDP proxy...')
    LOGGER.info(f'Src: {src}')
    LOGGER.info(f'Dst: {dst}')
    LOGGER.info(f'Loss Rate: {loss*100}%')
    LOGGER.info(f'Median Delay: {median_delay}ms')

    proxy_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    proxy_socket.bind(ip_to_tuple(src))

    client_address = None
    server_address = ip_to_tuple(dst)

    while True:
        data, address = proxy_socket.recvfrom(BUFFER_SIZE)
        LOGGER.debug(f"Received {len(data)} byte segment from {address}")
        seq_num, ack_num, msg_type, _, payload_size = unpack('!BBBBH', data[:6])
        LOGGER.debug(f"\ttype = {msg_type}, seq = {seq_num}, ack = {ack_num}, payload_len = {payload_size}")

        if client_address == None:
            client_address = address

        if address == client_address:
            simulate_lossy_send(proxy_socket, data, server_address, loss, median_delay)
        elif address == server_address:
            simulate_lossy_send(proxy_socket, data, client_address, loss, median_delay)
            #client_address = None
        else:
            LOGGER.warning('Unknown address: {}'.format(str(address)))
            client_address = address
            simulate_lossy_send(proxy_socket, data, server_address, loss, median_delay)


def ip_to_tuple(ip):
    """Parse IP string and return (ip, port) tuple.

    Arguments:
    ip -- IP address:port string. I.e.: '127.0.0.1:8000'.
    """
    ip, port = ip.split(':')
    return (ip, int(port))


def main():
    """Main method."""
    parser = argparse.ArgumentParser(description='Mock Network with Configurable Loss and Delay.')

    parser.add_argument('-s', '--src', required=True,
                        help='Source IP and port, i.e.: 127.0.0.1:8000')
    parser.add_argument('-d', '--dst', required=True,
                        help='Destination IP and port, e.g.: 127.0.0.1:8888')
    parser.add_argument('-l', '--loss', type=float, default=0.0,
                        help='Loss Rate, e.g.  0.1 for 10%')
    parser.add_argument('-m', '--delay', type=int, default=0,
                        help='Median delay (ms)')

    output_group = parser.add_mutually_exclusive_group()
    output_group.add_argument('-q', '--quiet', action='store_true', help='Be quiet')
    output_group.add_argument('-v', '--verbose', action='store_true', help='Be loud')

    args = parser.parse_args()

    if args.quiet:
        LOGGER.setLevel(logging.CRITICAL)
    if args.verbose:
        LOGGER.setLevel(logging.DEBUG)

    udp_proxy(args.src, args.dst, args.loss, args.delay)


if __name__ == '__main__':
    main()
