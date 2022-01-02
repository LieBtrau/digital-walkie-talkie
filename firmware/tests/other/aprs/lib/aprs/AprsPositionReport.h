#pragma once

#include "AprsPacket.h"
#include "AprsLocation.h"
#include "AprsSymbol.h"

/**
 * @brief Position and DF reports
 * According to chapter 8 POSITION AND DF REPORT DATA FORMATS of APRS 101
 */
class AprsPositionReport : public AprsPacket
{
public:
    AprsPositionReport(const byte *buffer, size_t len);
    AprsPositionReport(const AprsLocation& location, const std::string &comment);
    ~AprsPositionReport();
    AprsLocation getPosition() const;
    AprsSymbol getSymbol() const;
    void setLocation(const AprsLocation& location);
    void setSymbol(const AprsSymbol& symbol);
    const std::string encode() const;

private:
    AprsLocation _location;
    AprsSymbol _symbol;
};
