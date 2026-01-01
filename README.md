# Computer Networks Projects

A collection of networking projects completed as part of COMP 375 (Computer Networks) at the University of San Diego. These projects demonstrate practical implementation of core networking concepts including socket programming, protocol design, reliable data transport, and client-server architecture.

## Projects Overview

### 1. Reverse Engineering a Network Application
**Language:** C | **Focus:** Protocol Analysis, Socket Programming

Reverse-engineered a proprietary sensor network protocol by analyzing network traffic with Wireshark, then implemented a fully functional client from scratch.

**Key Accomplishments:**
- Analyzed raw network packets to decode an undocumented application-layer protocol
- Implemented TCP socket communication using POSIX sockets API
- Built a multi-stage authentication flow connecting through a proxy server to weather stations
- Parsed and formatted real-time sensor data (temperature, humidity, wind speed)

**Skills Demonstrated:** Wireshark, packet analysis, C socket programming, protocol reverse engineering

---

### 2. ToreroServe - A Multithreaded Web Server
**Language:** C++ | **Focus:** HTTP Protocol, Concurrent Programming

Built a fully functional HTTP/1.0 web server capable of serving static files, handling multiple concurrent connections, and generating dynamic directory listings.

**Key Accomplishments:**
- Implemented HTTP request parsing using regex for robust protocol compliance
- Developed a producer-consumer architecture with a bounded buffer for connection handling
- Created a thread pool (4 worker threads) for concurrent request processing
- Supported multiple MIME types (HTML, CSS, JS, images, PDF, JSON)
- Generated dynamic HTML for directory browsing with automatic index.html detection

**Skills Demonstrated:** HTTP protocol, multithreading, synchronization primitives, C++ STL, filesystem operations

---

### 3. Iterative DNS Resolver
**Language:** Python | **Focus:** DNS Protocol, Binary Parsing

Implemented an iterative DNS resolver that queries the DNS hierarchy starting from root servers to resolve A and MX records.

**Key Accomplishments:**
- Constructed and parsed DNS query/response messages at the byte level
- Implemented iterative resolution following referrals through the DNS hierarchy
- Handled DNS message compression (pointer-based name encoding)
- Supported both A (address) and MX (mail exchange) record types
- Built robust timeout and retry logic for unreliable network conditions

**Skills Demonstrated:** DNS protocol internals, UDP socket programming, binary data parsing, Python networking

---

### 4. Reliable Data Transport Protocol
**Language:** C++ | **Focus:** Transport Layer, Protocol Design

Designed and implemented a reliable data transport protocol over UDP, implementing core TCP-like reliability mechanisms.

**Key Accomplishments:**
- Implemented a three-way handshake for connection establishment
- Built stop-and-wait ARQ with alternating bit protocol for reliability
- Developed adaptive timeout using TCP's RTT estimation algorithm (Jacobson/Karels)
- Handled packet loss, duplication, and reordering scenarios
- Implemented graceful connection termination with proper state machine transitions

**Skills Demonstrated:** Transport layer protocols, reliability mechanisms, state machines, timeout management

---

### 5. Streaming Music Protocol
**Languages:** Java (Client) & C++ (Server) | **Focus:** Application Protocol Design, Multimedia Streaming

Implemented both client and server components of a custom streaming music protocol for real-time audio playback.

**Key Accomplishments:**
- Designed a binary application-layer protocol with typed message headers
- Implemented song library management and metadata queries
- Built chunked audio data transfer for streaming playback
- Handled network byte ordering (Big Endian) for cross-platform compatibility
- Created a Java audio client with threaded playback support

**Skills Demonstrated:** Protocol design, binary serialization, client-server architecture, cross-language interoperability

---

## Technical Skills Demonstrated

| Category | Technologies |
|----------|-------------|
| **Languages** | C, C++, Python, Java |
| **Networking** | TCP/IP, UDP, HTTP, DNS |
| **Concepts** | Socket Programming, Protocol Design, Reliable Transport, Multithreading |
| **Tools** | Wireshark, GDB, Valgrind, Make |
| **Paradigms** | Client-Server, Producer-Consumer, State Machines |

