//derived from sudomesh/LoRaLayer2 @ ^1.0.1

#pragma once
#include "Arduino.h"

#define BUFFERSIZE 16
#define MAX_PACKET_SIZE 255

struct BufferEntry {
  char data[MAX_PACKET_SIZE] = { 0 };
  size_t length = 0;
};

class packetBuffer {
  public:
    // use pointer to char instead? and then copy that raw data into a packet
    BufferEntry read();
    int write(BufferEntry entry);
    packetBuffer();
  private:
    BufferEntry buffer[BUFFERSIZE];
    int head;
    int tail;
};
