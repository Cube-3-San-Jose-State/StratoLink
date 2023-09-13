#include <Arduino.h>
#include <tuple>
#include "../lib/PA1616S.hpp"
#include "../lib/XBEE.hpp"
#include "../include/LOS-Calculations.hpp"
#include "../include/stratolink-dto.hpp"
#include "../include/mission-control-handler.hpp"
#include "../include/gimbal-system.hpp"

using namespace StratoLink;

Sat_Data sat_data;
// MissionControlHandler mission_control_handler;
// PA1616S GPS;
// MPL3115A2 barometer(17, 16);
// XBEE xbee(14, 15);
// LineOfSightCalculator los_calculator(GPS, barometer);
GimbalSystem gimbal_system;

String json_data = "";
int expected_heartbeat = 0;

// String GetSatData()
// {
//   try 
//   {
//     json_data = xbee.receiveData();
//     if (json_data.length() > 0)
//     {
//       Serial.println(json_data);
//       std::string json_std_string = json_data.c_str();
//       sat_data = mission_control_handler.ParseMissionControlData(json_std_string);
//       if (sat_data.heartbeat_count == expected_heartbeat)
//       {
//         expected_heartbeat++;
//         std::tuple<double, double, double> los_vector = los_calculator.CalculateLineofSightVector(sat_data);
//         gimbal_system.SetGimbalAngles(std::get<0>(los_vector), std::get<1>(los_vector), std::get<2>(los_vector));
//       }
//       else
//       {
//         throw std::runtime_error("Error: invalid heartbeat");
//       }
//     }
//   }
//   catch (const std::exception& e) 
//   {
//     Serial.println(e.what());
//   }

//   return json_data;
// }


    void DemoMovement() 
    {
        // Move to one extreme (far left and top)
        gimbal_system.RunGimbalMotorAngles(-90, 90); // assuming -90° is left and 90° is top for your setup
        
        delay(1000); // Wait for a second for viewers to see the position

        // Move to the opposite extreme (far right and bottom)
        gimbal_system.RunGimbalMotorAngles(90, -90); // assuming 90° is right and -90° is bottom for your setup

        delay(1000); // Wait for a second

        // Return to a neutral position (center and middle)
        gimbal_system.RunGimbalMotorAngles(0, 0); 
    }


void setup()
{
  Serial.begin(9600);
  // try
  // {
  //   // GPS.Initialize();
  //   // xbee.Initialize();
  //   // barometer.Initialize(10.0);
  // }
  // catch(const std::exception& e)
  // {
  //   Serial.println(e.what());
  //   // Consider stopping the execution or implementing a recovery procedure
  // }
}

void loop()
{
  DemoMovement();
  // Serial.println(GetSatData());
  delay(10000);
}
