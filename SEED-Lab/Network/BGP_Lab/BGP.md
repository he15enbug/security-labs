# BGP Exploration and Attack Lab
- *Border Gateway Protocol (BGP)* is the standard exterior gateway protocol designed to exchange routing and reachability information among autonomous systems (AS) on the Internet
- topics
    - how the BGP protocol works
    - autonomous systems
    - BGP configuration, BGP Large Communities
    - routing
    - Internet Exchange (IX)
    - BGP attack, network prefix hijacking

## Lab Setup
- the network map: each computer (hosts or routers) running inside the emulator is a docker container. The emulator also comes with a web application which visualizes all the hosts, routers, and networks. The URL is `http://localhost:8080/map.html`
- modifying the BGP configuration file: we need to modify the BGP configuration file in several tasks. We can do that by directly modifying the configuration file inside a container. Another way is to copy the file into the host VM, do the editing from the host VM, and then copy it back
    ```
    $ dockps | grep 180
    ...
    2967... as180r-router0-10.180.0.254
    $ docker cp 2967:/etc/bird/bird.conf ./as180_bird.conf <-- container to VM
    $ docker cp ./as180_bird.conf 2967:/etc/bird/bird.conf <-- VM to container
    $ docker exec 2967 birdc configure <-- run "birdc configure" on container
    ```

## Task 1: Stub Autonomous System
- an *autonomous system (AS)* is a collection of connected Internet Protocol (IP) routing prefixes under the control of one or more operators on behalf of a single administrative entity or domain. It is a basic unit in BGP. A stub AS is the type of AS that does not provide transit service to others. Most of end users are stub ASes, including universities, organizations, and most companies. Another type of AS is called transit AS. They provide transit services for other ASes, and they are Internet Service Providers (ISP)
- in this task, we focus on stub ASes, see how it peers with others

### Task 1.a: Understanding AS-155's BGP Configuration
- AS-155 is a stub AS, which has one network `10.155.0.0/24` and one BGP router `10.155.0.254`
- get a terminal on the container of this router, study its BGP configuration in `/etc/bird/bird.conf`
- *task 1.a.1*: from the BGP configuration file, identify who AS-155 peers with. We can ignore the filtering part of the configuration for now

## Task 2: Transit Autonomous System

## Task 3: Path Selection

## Task 4: IP Anycast

## Task 5: BGP Prefix Attack
