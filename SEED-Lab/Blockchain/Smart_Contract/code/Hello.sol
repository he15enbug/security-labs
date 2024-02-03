// SPDX-License-Identifier: MIT
pragma solidity ^0.8.15;

contract Hello{
    address public owner;
    uint256 counter;

    constructor(){
        owner = msg.sender;
    }

    // To make low level interactions using CALLDATA
    // there has to be fallback and receive function
    fallback() external payable {}
    receive() external payable {}

    function sayHello() public pure returns (string memory) {
        return "Hello World";
    }

    function getResult(uint a, uint b) public view returns (uint) {
        uint sum = a + b + counter;
        return sum;
    }

    function increaseCounter(uint k) public returns (uint) {
        counter += k;
        return counter;
    }
    function decreaseCounter(uint k) public returns (uint) {
        counter -= k;
        return counter;
    }

    function increaseCounter2(uint k) public payable returns (uint) {
        counter += k;
        return counter;
    }

    function getCounter() public view returns (uint) {
        return counter;
    }

    event MyMessage(address indexed _from, uint256 _value);
    function sendMessage() public returns (uint256) {
       emit MyMessage(msg.sender, counter);
       return counter;
    }
}
