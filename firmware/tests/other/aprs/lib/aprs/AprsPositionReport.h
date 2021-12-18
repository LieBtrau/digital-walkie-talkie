#pragma once

#include "libAprs.h"

/**
 * @brief Position and DF reports
 * According to chapter 8 POSITION AND DF REPORT DATA FORMATS of APRS 101
 */
class AprsPositionReport : public AprsPacket
{
private:
    char *latitude = nullptr;
    char *longitude = nullptr;
    byte symbolTableId = 0;
    byte symbolCode = 0;

public:
    AprsPositionReport(const byte *buffer, size_t len);
    ~AprsPositionReport();
    const char *getLatitude();
    const char *getLongitude();
    byte getSymbolTableId();
    byte getSymbolCode();
    char* encode();
};