//based on : sudomesh/LoRaLayer2 @ ^1.0.1

#include "Layer2.h"

Layer2::Layer2(Layer1Class *radio) : _radio(radio)
{
    _rxBuffer = new packetBuffer;
}

Layer2::~Layer2()
{
}

/* Layer 3 tx/rx wrappers
*/
int Layer2::writeData(Datagram datagram, size_t length)
{
#ifdef LL2_DEBUG
    Serial.printf("LoRaLayer2::writeData(): datagram.message = ");
    for (int i = 0; i < length - 5; i++)
    {
        Serial.printf("%c", datagram.message[i]);
    }
    Serial.printf("\r\n");
#endif
    Packet packet = buildPacket(datagram, length);
    return writeToBuffer(_radio->txBuffer, packet);
}

Packet Layer2::readData()
{
    Packet packet = readFromBuffer(_rxBuffer);
#ifdef LL2_DEBUG
    if (packet.totalLength > 0)
    {
        Serial.printf("LoRaLayer2::readData(): packet.datagram.message = ");
        for (int i = 0; i < packet.totalLength - HEADER_LENGTH - 5; i++)
        {
            Serial.printf("%c", packet.datagram.message[i]);
        }
        Serial.printf("\r\n");
    }
#endif
    return packet;
}

/* private wrappers for packetBuffers
*/
int Layer2::writeToBuffer(packetBuffer *buffer, Packet packet)
{
    BufferEntry entry;
    memcpy(&entry, (uint8_t*)&packet, packet.totalLength);
    entry.length = packet.totalLength;
    int ret = buffer->write(entry);
    int length = _radio->transmit();
    if (length > 0)
    {
#ifdef LL2_DEBUG
        Serial.printf("LL2::daemon(): transmitted packet of length: %d\r\n", length);
#endif
        _messageCount = (_messageCount + 1) % 256;
    }
    return ret;
}

Packet Layer2::readFromBuffer(packetBuffer *buffer)
{
    Packet packet;
    BufferEntry entry = buffer->read();
    memcpy(&packet, &entry.data[0], sizeof(packet));
    return packet;
}

Packet Layer2::buildPacket(Datagram datagram, size_t length)
{
    Packet packet;
    packet.totalLength = HEADER_LENGTH + length;
    memcpy(packet.sender, _localAddress, ADDR_LENGTH);
    memcpy(packet.receiver, datagram.destination, ADDR_LENGTH); //should be a layer2 address
    packet.sequence = messageCount();
    memcpy(&packet.datagram, &datagram, length);
    return packet;
}

int Layer2::daemon()
{
    // see if there are any packets to be received
    // first check if any interrupts have been set,
    // then check if any packets have been added to Layer1 rxbuffer
    if (_radio->receive() > 0)
    {
#ifdef LL2_DEBUG
        Serial.printf("LL2Class::daemon: received packet\r\n");
#endif
        receive();
    }
    return 0;
}

/* Receive and decide function
*/
void Layer2::receive()
{
    Packet packet = readFromBuffer(_radio->rxBuffer);
    //BufferEntry entry = LoRa1->rxBuffer.read();
    //memcpy(&packet, entry.data, entry.length);

#ifdef LL2_DEBUG
    Serial.printf("LoRaLayer2::receive(): packet.datagram.message = ");
    for (int i = 0; i < packet.totalLength - HEADER_LENGTH - 5; i++)
    {
        Serial.printf("%c", packet.datagram.message[i]);
    }
    Serial.printf("\r\n");
#endif
    if (packet.totalLength > 0)
    {
        writeToBuffer(_rxBuffer, packet);
    }
    //else buffer is empty, do nothing;
    return;
}

/* Public access to local variables
*/
uint8_t Layer2::messageCount()
{
    return _messageCount;
}

