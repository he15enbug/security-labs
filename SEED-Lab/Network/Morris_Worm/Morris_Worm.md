# Morris Worm
- write a simple worm and test it in an Internet emulator
- worms involve two main part: attack and self-duplication. The attack part exploits a vulnerability (or a few of them), so a worm can get entry to another computer. The self-duplication part is to send a copy of itself to the compromised machine, and then launch the attack from there
- topics covered:
    - buffer-overflow
    - worm's self-duplication and propagation behavior
    - the SEED Internet emulator
    - network tools
- prerequisite: buffer-overflow attack lab (server version)

## Labsetup
- use the smaller one emulator `Labsetup/internet-nano`
- there is a map of the emulated internet at `http://localhost:8080/map.html`, we need to go to `Labsetup/map` folder, build and start the Map container
