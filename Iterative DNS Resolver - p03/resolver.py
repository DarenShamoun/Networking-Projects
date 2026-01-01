"""
Module: resolver

A program used to perform iterative DNS queries for A or MX records.
"""
import socket
from struct import pack, unpack
from argparse import ArgumentParser
from typing import Optional
import logging
from random import randrange

from helpers import *


def parse_record(response: bytes, start_index: int) -> tuple[DNSRecord,int]:
    """
    Creates a DNS record form the data in response starting at the given
    index, along with the index where the record's info ends.

    Args:
        response (bytes): The response that will contain the record.
        start_index (int): The location in the response where the record starts.

    Return:
        tuple[DNSRecord,int]: A tuple containing the DNS record that was found
        starting at index, and the index immediately after the end of that
        record.
    """

    pass # TODO: implement this function (see the development plan in the project specifications


def parse_response(response: bytes, expected_query_id: int) -> Optional[DNSResponse]:
    """
    Parses the given response, returning a DNSResponse object containing all
    the answer, authority, and additional records. If response's ID doesn't
    match the expected ID, then return None.

    Args:
        response (bytes): The response to parse
        expected_query_id (int): The ID of the original query.

    Returns:
        Optional[DNSResponse]: A parsed version of the response, or None if
        the response wasn't for the original query (i.e. had the wrong ID).
    """

    # step 1: unpack header and check that query ID is correct

    pass # TODO: implement step 1 (see the specifications document for details)

    # step 2: parse the Question section

    pass # TODO: implement step 2 (see the specifications document for details)

    # step 3: Parse the Answer, Authority, and Additional sections

    pass # TODO: implement step 3 (see the specifications document for details)


def query_servers(query: bytes, servers: list[str]) -> Optional[bytes]:
    """
    Sends the given query to the servers, one at a time, stopping once it gets
    a response from one of them. Returns None if all of the servers timeout.

    Args:
        query (bytes): The message to send to the servers.
        servers (list[str]): A list of IP addresses for the servers.

    Returns:
        Optional[bytes]: The data received from one of the servers, or None if
        none of the servers responded.
    """

    pass # TODO: implement this function (see the specifications document)


def locate_answer(hostname: str, is_mx: bool, response_records: DNSResponse) -> Optional[str]:
    """
    Uses the given response to resolve the query (with the given id) for the given hostname.

    Args:
        hostname (string): The name of the host to resolve.
        is_mx (boolean): True if requesting the MX record result, False if
          requesting the A record.
        response_record (DNSResponse): The parsed response to be examined.

    Returns:
        Optional[str]: A string representation of an IP address (e.g. "192.168.0.1") or
          mail server (e.g. "mail.google.com"). If the request could not be
          resolved, None will be returned.
    """
    logging.info("Looking for answer in response.")
    logging.debug(response_records)

    pass # TODO: implement this function (see the specifications document)


def resolve(hostname: str, servers: list[str], is_mx: bool=False) -> Optional[str]:
    """
    Returns a string with the IP address (for an A record) or name of mail
    server associated with the given hostname.

    Args:
        hostname (string): The name of the host to resolve.
        servers (list[str]): List of IP addresses for DNS servers to query
        is_mx (boolean): True if requesting the MX record result, False if
          requesting the A record.

    Returns:
        Optional[string]: A string representation of an IP address (e.g. "192.168.0.1") or
          mail server (e.g. "mail.google.com"). If the request could not be
          resolved, None will be returned.
    """
    logging.info(f"Resolving {hostname} (type={'A' if not is_mx else 'MX'}) using the following servers: {servers}")

    # Step 1: Use the construct_query function to create a query of the
    # appropriate type, using a randomly generated ID between 0 and 65535 (i.e. 2^16-1)

    pass # TODO: implement step 1 (see the specifications document for details)

    # Step 2: Send the query to the servers

    pass # TODO: implement step 2 (see the specifications document for details)

    # Step 3: Parse the response

    pass # TODO: implement step 3 (see the specifications document for details)

    # Step 4: Use the locate_function to determine the answer to the query

    pass # TODO: implement step 4 (see the specifications document for details)

    return None # TODO: remove this when you are done implementing this function


def main() -> None:
    """ Parses command line arguments and calls resolver based on the
    specified type of query, displaying the result to the user. """

    # parse the command line arguments
    parser = ArgumentParser(prog="resolver.py",
                            description="An iterative DNS resolver for A and MX queries.")
    parser.add_argument('name', help="The hostname to resolve.")
    parser.add_argument('-m', '--mx', action='store_true',
                        help="Perform an MX instead of an A query.")
    parser.add_argument('-v', '--verbose', action='store_true',
                        help="Print detailed program output to screen.")
    args = parser.parse_args()

    setup_logging(args.verbose)


    roots = get_root_servers()
    answer = resolve(args.name, roots, is_mx=args.mx)

    if answer is not None:
        if args.mx:
            print(f"Mail Server for {args.name}: {answer}")
        else:
            print(f"IP address for {args.name}: {answer}")
    else:
        print("ERROR: Could not resolve request.")


def setup_logging(verbose_output: bool) -> None:
    """ Sets up logging to a file (output.log) as well as to the screen.

    Args:
        verbose_output (bool): True if logger should print DEBUG level
        messages to screen, False to print WARNING level and above only.
    """
    log = logging.getLogger()

    log.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(levelname)s: %(message)s')

    # set up logging to the output.log file
    fh = logging.FileHandler('output.log', mode='w', encoding='utf-8')
    fh.setLevel(logging.DEBUG)
    fh.setFormatter(formatter)
    log.addHandler(fh)

    # set up logging to the screen, based on the verbosity level set by user
    ch = logging.StreamHandler()
    if verbose_output:
        ch.setLevel(logging.DEBUG)
    else:
        ch.setLevel(logging.WARNING)
    log.addHandler(ch)


if __name__ == "__main__":
    main()
