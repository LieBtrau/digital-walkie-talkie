#pragma once
#include "Ax25Client.h"
#include "AsyncDelay.h"

class AprsClient
{
public:
    AprsClient(Ax25Client &ax25client);
    ~AprsClient();
    int sendMessage(Ax25Callsign &destination, std::string& message, bool ackRequired = false);
    bool sendLocation(float latitude, float longitude);
    void setLocationReceivedCallback(void (*callback)(const Ax25Callsign &sender, float latitude, float longitude));
    void setMessageReceivedCallback(void (*callback)(const std::string& addressee, const std::string& message));
    void setAckReceivedCallback(void (*callback)(int messageId));
    void receiveFrame(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length);
    void loop();

private:
    const byte MAX_TX_RETRIES = 7;
    Ax25Client *_ax25Client;
    std::string _info_field = "";
    int _messageCounter = 0;
    byte _sendTrialCounter = 0;
    AsyncDelay _resendTimer;
    void (*_locationReceivedCallback)(const Ax25Callsign &sender, float latitude, float longitude) = nullptr;
    void (*_messageReceivedCallback)(const std::string& addressee, const std::string& message) = nullptr;
    void (*_ackReceivedCallback)(int messageId) = nullptr;
    bool sendMessage();
};