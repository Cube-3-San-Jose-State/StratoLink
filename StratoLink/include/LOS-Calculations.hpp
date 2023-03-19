#pragma once
#include <Arduino.h>
#include <tuple>
#include "../lib/PA1616S.hpp"
#include "../include/stratolink-dto.hpp"
#include "../lib/MPL3115A2.hpp"

namespace StratoLink
{

    class LineOfSightCalculator
    {

    private:
        Sat_Data sat_data;
        PA1616S _gps;
        MPL3115A2 _barometer;

    public:
        LineOfSightCalculator(PA1616S gps, MPL3115A2 barometer)
            : _gps(gps), _barometer(barometer)
        {
        }

        std::tuple<double, double, double> latLonAltToECEF(double lat, double lon, double alt)
        {
            const double axis = 6378137.0;             // Semi-major axis of the Earth (meters)
            const double eccentricity = 0.08181919084; // Eccentricity of the Earth's ellipsoid

            double lat_rad = radians(lat);
            double lon_rad = radians(lon);
            double N = axis / sqrt(1 - eccentricity * eccentricity * sin(lat_rad) * sin(lat_rad));

            double x = (N + alt) * cos(lat_rad) * cos(lon_rad);
            double y = (N + alt) * cos(lat_rad) * sin(lon_rad);
            double z = ((1 - eccentricity * eccentricity) * N + alt) * sin(lat_rad);

            return std::make_tuple(x, y, z);
        }

        std::tuple<double, double, double> CalculateLineofSightVector(Sat_Data sat_data)
        {
            _gps.Update();
            _barometer.Update();
            double device_lat = _gps.GetLatitude();
            double device_lon = _gps.GetLongitude();
            double device_alt = _barometer.ReadAltitude();

            // Convert device and satellite coordinates to ECEF
            double device_x, device_y, device_z;
            std::tie(device_x, device_y, device_z) = latLonAltToECEF(device_lat, device_lon, device_alt);

            double sat_x, sat_y, sat_z;
            double sat_alt = sat_data.barometer_data.altitude;
            std::tie(sat_x, sat_y, sat_z) = latLonAltToECEF(sat_data.gps_data.latitude, sat_data.gps_data.longitude, sat_alt);

            // Calculate the line-of-sight vector
            double los_vector_x = sat_x - device_x;
            double los_vector_y = sat_y - device_y;
            double los_vector_z = sat_z - device_z;

            return std::make_tuple(los_vector_x, los_vector_y, los_vector_z);
        }
    };

}