#pragma once
#include "Ax25Client.h"
#include "AsyncDelay.h"

class AprsClient
{
public:
    AprsClient(Ax25Client &ax25client);
    ~AprsClient();
    int sendMessage(Ax25Callsign &destination, const char *message, bool ackRequired = false);
    bool sendLocation(float latitude, float longitude);
    void setLocationReceivedCallback(void (*callback)(const Ax25Callsign &sender, float latitude, float longitude));
    void setMessageReceivedCallback(void (*callback)(const char *addressee, const char *message));
    void setAckReceivedCallback(void (*callback)(int messageId));
    void receiveFrame(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length);
    void loop();

private:
    const byte MAX_TX_RETRIES = 7;
    Ax25Client *_ax25Client;
    char _info_field[100];
    int _messageCounter = 0;
    byte _sendTrialCounter = 0;
    AsyncDelay _resendTimer;
    void (*_locationReceivedCallback)(const Ax25Callsign &sender, float latitude, float longitude) = nullptr;
    void (*_messageReceivedCallback)(const char *addressee, const char *message) = nullptr;
    void (*_ackReceivedCallback)(int messageId) = nullptr;
    bool sendMessage();
};