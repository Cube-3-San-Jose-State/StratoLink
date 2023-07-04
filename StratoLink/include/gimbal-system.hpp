#pragma once
#include <tuple>
#include <AccelStepper.h> // Include the AccelStepper library
#include "../include/LOS-Calculations.hpp"

namespace StratoLink
{
    class GimbalSystem
    {
    private:
        const int XstepPin = 2;
        const int XdirPin = 3;
        const int YstepPin = 4;
        const int YdirPin = 5;
        const int AstepPin = 6;
        const int AdirPin = 7;

        AccelStepper motor_rotation = AccelStepper(AccelStepper::DRIVER, XstepPin, XdirPin);
        AccelStepper motor_pitch1 = AccelStepper(AccelStepper::DRIVER, YstepPin, YdirPin);
        AccelStepper motor_pitch2 = AccelStepper(AccelStepper::DRIVER, AstepPin, AdirPin);


    public:
        GimbalSystem()
        {
            motor_rotation.setMaxSpeed(200);
            motor_rotation.setAcceleration(100);
            motor_pitch1.setMaxSpeed(200);
            motor_pitch1.setAcceleration(100);
            motor_pitch2.setMaxSpeed(200);
            motor_pitch2.setAcceleration(100);
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
            RunGimbalMotorAngles(azimuth_degrees, elevation_degrees);
        }

        void RunGimbalMotorAngles(double azimuth_degrees, double elevation_degrees)
        {
            motor_rotation.moveTo(azimuth_degrees);
            motor_pitch1.moveTo(elevation_degrees);
            motor_pitch2.moveTo(elevation_degrees);

            // Run the motors until they reach their target positions
            while (motor_rotation.isRunning() || motor_pitch1.isRunning() && motor_pitch2.isRunning())
            {
                motor_rotation.run();
                motor_pitch1.run();
                motor_pitch2.run();
            }
        }
    };
}
