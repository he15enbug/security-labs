# The Mitnick Attack Lab
- the Mitnick attack is a special type of TCP session hijacking
- topics
    - TCP session hijacking
    - TCP three-way handshake protocol
    - Remote shell `rsh`

## How the Mitnick Attack Works
- instead of hijacking existing TCP connection between two victims A and B, the Mitnick attack create a TCP connection between A and B first on their behalf, and then naturally hijacks the connection

- in the actual Mitnick attack, host A was called X-Terminal, which was the target. Mitnick wanted to log into X-Terminal and run his commands on it. Host B was a trusted server, which was allowed to log into X-Terminal without a password. In order to log into X-Terminal, Mitnick had to impersonate the trusted server, so he did not need to provide any password

- here are 4 primary steps in this attack
    1. Sequence number prediction
    2. SYN flooding attack on the trusted server
    3. Spoofing a TCP connection
    4. Running a remote shell

## Lab Setup

## Task 1: Simulated SYN flooding

## Task 2: Spoof TCP Connections and `rsh` Sessions

## Task 3: Set Up a Backdoor
