#pragma once
#include <tuple>
#include "../include/LOS-Calculations.hpp"

namespace StratoLink
{
    class GimbalSystem
    {

    public:
        GimbalSystem()
        {
        }

        void SetGimbalAngles(double x, double y, double z)
        {
            // Calculate the azimuth angle (rotation around the Z-axis)
            double azimuth = atan2(y, x);

            // Calculate the elevation angle (rotation around the Y-axis)
            double range_xy = sqrt(x * x + y * y);
            double elevation = atan2(z, range_xy);

            // Convert radians to degrees
            double azimuth_degrees = degrees(azimuth);
            double elevation_degrees = degrees(elevation);

            // Set the gimbal angles
            // Assuming you have a function called SetGimbalMotorAngles, which takes azimuth and elevation angles in degrees
            SetGimbalMotorAngles(azimuth_degrees, elevation_degrees);
        }

        // Implement the SetGimbalMotorAngles function here to control your gimbal motors
        void SetGimbalMotorAngles(double azimuth_degrees, double elevation_degrees)
        {

            // Your code to control the gimbal motors goes here
        }
    };

}