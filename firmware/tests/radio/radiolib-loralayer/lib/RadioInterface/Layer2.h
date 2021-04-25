//based on : sudomesh/LoRaLayer2 @ ^1.0.1

#pragma once
#include <packetBuffer.h>
#include "Arduino.h"
#include "Layer1.h"

#define ADDR_LENGTH 4
#define MESSAGE_LENGTH 233
#define HEADER_LENGTH 17

struct Datagram
{
    uint8_t destination[ADDR_LENGTH];
    uint8_t type;
    uint8_t message[MESSAGE_LENGTH];
};

struct Packet
{
    uint8_t totalLength;
    uint8_t sender[ADDR_LENGTH];
    uint8_t receiver[ADDR_LENGTH];
    uint8_t sequence; // message count of packets tx by sender
    Datagram datagram;
};

class Layer2
{
private:
    int writeToBuffer(packetBuffer *buffer, Packet packet);
    Packet readFromBuffer(packetBuffer *buffer);
    Packet buildPacket(Datagram datagram, size_t length);
    void receive();
    Layer1 *_radio;
    packetBuffer *_rxBuffer; // L2 sending to L3
    uint8_t _messageCount=0;
    uint8_t _localAddress[ADDR_LENGTH];

public:
    Layer2(Layer1 *radio);
    ~Layer2();
    // Layer 3 tx/rx wrappers
    int writeData(Datagram datagram, size_t length);
    Packet readData();
    int daemon();
    // Public access to local variables
    uint8_t messageCount();
};
