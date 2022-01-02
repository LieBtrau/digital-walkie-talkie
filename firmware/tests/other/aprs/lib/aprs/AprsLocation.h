#pragma once
#include "Arduino.h"


/**
 * @brief Deal with APRS-coordinates
 * APRS sucks in more than one sense.
 *  - The decimal point is between the minutes and the seconds, not between degrees and minutes.
 *  - minutes are expressed as values ranging 0 to 59, while seconds are expressed as hundredths of a minute (Huh??).  Why the difference??
 * It would have made much more sense to express latitude and longitude as decimal values instead of brewing some mix of units.
 */
class AprsLocation
{
private:
    double _latitude = 0;  //!< unit = decimal degrees, <0 southern hemisphere, >0 northern hemisphere
    double _longitude = 0; //!< unit = decimal degrees, <0 western hemisphere, >0 eastern hemisphere
public:
    AprsLocation();
    AprsLocation(double latitude, double longitude);
    AprsLocation &operator=(const AprsLocation &loc);
    void setLocation(double latitude, double longitude);
    void setLatitude(std::string encodedLatitude);
    void setLongitude(std::string encodedLongitude);
    double getLatitude();
    double getLongitude();
    std::string encodeLatitude() const;
    std::string encodeLongitude() const;
};