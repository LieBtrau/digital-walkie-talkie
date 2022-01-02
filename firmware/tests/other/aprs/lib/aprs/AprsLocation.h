#pragma once
#include "Arduino.h"

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
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    double getLatitude();
    double getLongitude();
    std::string encodeLatitude() const;
    std::string encodeLongitude() const;
};