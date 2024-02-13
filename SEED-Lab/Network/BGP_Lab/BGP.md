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

## Task 2: Transit Autonomous System

## Task 3: Path Selection

## Task 4: IP Anycast

## Task 5: BGP Prefix Attack
