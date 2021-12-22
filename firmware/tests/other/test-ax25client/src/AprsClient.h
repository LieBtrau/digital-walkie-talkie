#pragma once
#include "Ax25Client.h"

class AprsClient
{
public:
    AprsClient(Ax25Client &ax25client);
    ~AprsClient();
    bool sendMessage(Ax25Callsign &destination, const char *message);
    bool sendLocation(float latitude, float longitude);
    void setLocationReceivedCallback(void (*callback)(const Ax25Callsign &sender, float latitude, float longitude));
    void setMessageReceivedCallback(void (*callback)(const char* addressee, const char *message));
    void receiveFrame(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length);

private:
    Ax25Client *_ax25Client;
    int _messageCounter = 0;
    void (*_locationReceivedCallback)(const Ax25Callsign &sender, float latitude, float longitude);
    void (*_messageReceivedCallback)(const char* addressee, const char *message);
};