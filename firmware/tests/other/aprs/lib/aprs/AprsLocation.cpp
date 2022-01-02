/**
 * @file AprsLocation.cpp
 * @author Christoph Tack (christoph.j.f.tack at gmail dot com)
 * @brief Handling of APRS-position coordinates and converting them to other formats
 * Conversion has been checked using : https://coordinates-converter.com/en
 *  21.869167N, 33.747778E;     //Bir Tawil
 *  41.89332N, 12.482932E;      //Roma
 *  -6.175394N, 106.827183E;    //Jakarta
 *  34.053691, -118.242767;     //Los Angeles
 *  -54.806933, -68.307325;     //Ushuaia
 * @version 0.1
 * @date 2022-01-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
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

void AprsLocation::setLatitude(std::string encodedLatitude)
{
    Serial.printf("%s\r\n", encodedLatitude.c_str());
    double degrees = strtod(encodedLatitude.substr(0, 2).c_str(), nullptr);
    double minutes = strtod(encodedLatitude.substr(2, 2).c_str(), nullptr) / 60;
    double seconds = strtod(encodedLatitude.substr(5, 2).c_str(), nullptr) / 10000;
    _latitude = degrees + minutes + seconds;
    if (encodedLatitude.at(6) == 'S')
    {
        _latitude = -_latitude;
    }
    assert(_latitude >= -90 && _latitude <= 90);
}

void AprsLocation::setLongitude(std::string encodedLongitude)
{
    Serial.printf("%s\r\n", encodedLongitude.c_str());
    double degrees = strtod(encodedLongitude.substr(0, 3).c_str(), nullptr);
    double minutes = strtod(encodedLongitude.substr(3, 2).c_str(), nullptr) / 60;
    double seconds = strtod(encodedLongitude.substr(6, 2).c_str(), nullptr) / 10000;
    _longitude = degrees + minutes + seconds;
    if (encodedLongitude.at(7) == 'W')
    {
        _longitude = -_longitude;
    }
    assert(_longitude >= -180 && _longitude <= 180);
}

void AprsLocation::setLocation(double latitude, double longitude)
{
    assert(latitude >= -90 && latitude <= 90);
    assert(longitude >= -180 && longitude <= 180);
    _latitude = latitude;
    _longitude = longitude;
}

AprsLocation &AprsLocation::operator=(const AprsLocation &loc)
{
    setLocation(loc._latitude, loc._longitude);
    return (*this);
}

std::string AprsLocation::encodeLatitude() const
{
    char buf[20];
    double degrees, minutes;
    double latitude = _latitude > 0 ? _latitude : -_latitude;
    double remainder = modf(latitude, &degrees);
    remainder = modf(remainder * 60, &minutes);
    double seconds = remainder * 100;
    sprintf(buf, "%02.0f%02.0f.%02.0f%c", degrees, minutes, seconds, _latitude > 0 ? 'N' : 'S');
    return std::string(buf);
}

std::string AprsLocation::encodeLongitude() const
{
    char buf[20];
    double degrees, minutes;
    double longitude = _longitude > 0 ? _longitude : -_longitude;
    double remainder = modf(longitude, &degrees);
    remainder = modf(remainder * 60, &minutes);
    double seconds = remainder * 100;
    sprintf(buf, "%03.0f%02.0f.%02.0f%c", degrees, minutes, seconds, _longitude > 0 ? 'E' : 'W');
    return std::string(buf);
}