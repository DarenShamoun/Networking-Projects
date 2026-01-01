"""
Module: helpers

A set of classes and functions used in Project 3 (DNS Resolver).
"""
from enum import Enum
from dataclasses import dataclass, field
from typing import Optional
import struct


class DNSRecordType(Enum):
    A = 1
    CNAME = 5
    AAAA = 28
    MX = 15
    NS = 2
    SOA = 6


@dataclass(frozen=True)
class DNSRecord:
    """ Representation of a DNS record, sans TTL. """
    name: str
    type: DNSRecordType
    value: str | list[int]

    def value_string(self):
        """ Returns a string representation of self.value. """
        if isinstance(self.value, str):
            return self.value
        elif len(self.value) == 4:
            return ".".join([str(o) for o in self.value])
        else:
            return ":".join([str(o) for o in self.value])

    def __str__(self):
        return f"({self.type.name}, name={self.name}, value={self.value_string()})"


@dataclass(frozen=True)
class DNSResponse:
    """ Representation of a DNS response. """
    query_name: str
    query_type: DNSRecordType
    answers: list[DNSRecord] = field(default_factory=list)
    authorities: list[DNSRecord] = field(default_factory=list)
    additional: list[DNSRecord] = field(default_factory=list)

    def __str__(self):
        s = f"Query: {self.query_name} (type: {self.query_type.name})\n"
        for name, records in [("Answers", self.answers),
                              ("Authorities", self.authorities),
                              ("Additional", self.additional)]:
            if len(records) > 0:
                s += f"{name} ({len(records)})\n"
                for r in records:
                    s += f"  {r}\n"

        return s

    def get_answer(self, target_name: str, target_type: DNSRecordType) -> Optional[DNSRecord]:
        """
        Finds the record in the answers or additional sections that match the
        target name and type, or None if there are no matches.

        Args:
            target_name (str): The string to match in the record's name field
            target_type (DNSRecordType): The type to match in the record's type field

        Returns:
            Optional[DNSRecord]: The DNS record matching the target name and
            type or None if there was no matching records.
        """
        if len(self.answers) == 0 and len(self.additional) == 0:
            return None

        answer_matches = [record for record in self.answers
                          if (record.type == target_type and record.name == target_name)]

        if len(answer_matches) > 0:
            return answer_matches[0]

        additional_matches = [record for record in self.additional
                              if (record.type == target_type and record.name == target_name)]

        if len(additional_matches) > 0:
            return additional_matches[0]

        return None


def get_dns_name_encoding(name: str) -> bytes:
    """
    Gets the DNS name encoding of the given name string.

    Args:
        name (string): the string to convert

    Returns:
        bytes: The network formatted string

    >>> get_dns_name_encoding('www.sandiego.edu')
    b'\\x03www\\x08sandiego\\x03edu\\x00'
    >>> get_dns_name_encoding('google.com')
    b'\\x06google\\x03com\\x00'
    """
    parts = name.split('.')
    encoded_name = b""
    for item in parts:
        format_string = f"B{len(item)}s"
        encoded_name += struct.pack(format_string, len(item), item.encode())
    encoded_name += struct.pack("B", 0)
    return encoded_name


def decode_dns_name(buffer: bytes, start: int) -> tuple[str,int]:
    """
    Decodes the DNS name starting at index <start> in <buffer>.

    Args:
        buffer (bytes): the entire network buffer message
        start (int): the location within the message where the network string
            starts.

    Returns:
        A (str, int) tuple
            - str: The human readable string.
            - int: The index one past the end of the string, i.e. the starting
              index of the value immediately after the string.

    >>> network_str = get_dns_name_encoding('www.sandiego.edu')
    >>> decode_dns_name(network_str, 0)
    ('www.sandiego.edu', 18)
    """

    toReturn = ""
    position = start
    length = -1
    while True:
        length = struct.unpack("!B", buffer[position:position+1])[0]
        if length == 0:
            position += 1
            break

        # Handle DNS pointers (!!)
        elif (length & 1 << 7) and (length & 1 << 6):
            b2 = struct.unpack("!B", buffer[position+1:position+2])[0]
            offset = 0
            """
            # strip off leading two bits shift by 8 to account for "length"
            # being the most significant byte
            ooffset += (length & 1 << i)ffset += (length & 0x3F) << 8  

            offset += b2
            """
            for i in range(6) :
                offset += (length & 1 << i) << 8
            for i in range(8):
                offset += (b2 & 1 << i)
            dereferenced = decode_dns_name(buffer, offset)[0]
            return toReturn + dereferenced, position + 2

        format_string = str(length) + "s"
        position += 1
        toReturn += struct.unpack(format_string, buffer[position:position+length])[0].decode()
        toReturn += "."
        position += length

    return toReturn[:-1], position


def construct_query(query_id: int, hostname: str,
                    query_type: DNSRecordType=DNSRecordType.A) -> bytes:
    """
    Constructs a DNS query message for a given hostname and query_id.

    Args:
        query_id (int): The ID for the query
        hostname (string): Target for the query (e.g. www.sandiego.edu)
        query_type (DNSRecordType): The type of record we are requesting.

    Returns:
        bytes: Binary representation of the DNS query message
    """
    flags = 0 # 0 implies basic iterative query

    # one question, no answers for basic query
    num_questions = 1
    num_answers = 0
    num_auth = 0
    num_additional = 0

    # "!HHHHHH" means pack 6 Half integers (i.e. 16-bit values) into a single
    # string, with data placed in network order (!)
    header = struct.pack("!HHHHHH", query_id, flags, num_questions, num_answers, num_auth,
                  num_additional)

    qname = get_dns_name_encoding(hostname)
    qtype = query_type.value # request A type
    remainder = struct.pack("!HH", qtype, 1)
    query = header + qname + remainder
    return query


def get_root_servers() -> list[str]:
    """ Returns a list of the root servers IP addresses in string format. """ 
    server_ips = []
    with open('root-servers.txt', 'r') as root_server_info:
        for line in root_server_info:
            server_ips.append(line.strip())

    return server_ips
