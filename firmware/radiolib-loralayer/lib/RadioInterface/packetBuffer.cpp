#include "packetBuffer.h"
#include <cstring>

/* Fifo Buffer Class
*/
packetBuffer::packetBuffer() : head(0),
                               tail(0){};

// reads a packet from buffer
BufferEntry packetBuffer::read()
{
    BufferEntry entry;
    if (head == tail)
    {
        // if buffer empty, return empty entry
        return entry;
    }
    // otherwise return entry from tail
    tail = (tail + 1) % BUFFERSIZE;
    memcpy(&entry, &buffer[tail], sizeof(entry));
    // clear data stored in tail of buffer
    buffer[tail].length = 0;
    memset(buffer[tail].data,0,MAX_PACKET_SIZE);
    return entry;
}

// writes a packet to buffer
int packetBuffer::write(BufferEntry entry)
{
    int ret = BUFFERSIZE;
    BufferEntry empty;
    // if full, return the size of the buffer
    if (((head + 1) % BUFFERSIZE) != tail)
    {
        head = (head + 1) % BUFFERSIZE;
        // clear any previous data stored in buffer
        buffer[head] = empty;
        // copy new data into buffer
        buffer[head] = entry;
        ret = head - tail; // and return the packet's place in line
    }
    // if full, return the size of the buffer
    return ret;
}
