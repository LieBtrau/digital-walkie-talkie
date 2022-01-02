#include "AprsLocation.h"
#include "assert.h"

AprsLocation::AprsLocation() : _latitude(-100), _longitude(-200) {}

AprsLocation::AprsLocation(double latitude, double longitude)
{
    setLocation(latitude, longitude);
}

double AprsLocation::getLatitude()
{
    assert(_latitude >= -90 && _latitude <= 90);
    return _latitude;
}

double AprsLocation::getLongitude()
{
    assert(_longitude >= -180 && _longitude <= 180);
    return _longitude;
}
void AprsLocation::setLatitude(double latitude)
{
    assert(latitude >= -90 && latitude <= 90);
    _latitude = latitude;
}

void AprsLocation::setLongitude(double longitude)
{
    assert(longitude >= -180 && longitude <= 180);
    _longitude = longitude;
}

void AprsLocation::setLocation(double latitude, double longitude)
{
    setLatitude(latitude);
    setLongitude(longitude);
}

AprsLocation &AprsLocation::operator=(const AprsLocation &loc)
{
    setLocation(loc._latitude, loc._longitude);
    return (*this);
}

std::string AprsLocation::encodeLatitude() const
{
    char buf[20];
    sprintf(buf, "%07.2f%c", abs(_latitude) * 100, _latitude > 0 ? 'N' : 'S');
    return std::string(buf);
}

std::string AprsLocation::encodeLongitude() const
{
    char buf[20];
    sprintf(buf, "%08.2f%c", abs(_longitude) * 100, _longitude > 0 ? 'E' : 'W');
    return std::string(buf);
}