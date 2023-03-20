#pragma once
#include "../include/stratolink-dto.hpp"

namespace StratoLink
{
    class MissionControlHandler
    {
    public:
        String FormatContainerData(Sat_Data data)
        {
            char request_parameter[500];
            snprintf(
                request_parameter, 500, kResponseBodyFormat,
                data.id,
                data.heartbeat_count,
                data.payload_deployed,
                data.flight_mode,
                data.gps_data.latitude, data.gps_data.longitude,
                data.barometer_data.altitude, data.barometer_data.temperature,
                data.imu_data.acceleration_x, data.imu_data.acceleration_y, data.imu_data.acceleration_z,
                data.imu_data.gyro_x, data.imu_data.gyro_y, data.imu_data.gyro_z,
                data.imu_data.pitch, data.imu_data.roll);
            return request_parameter;
        }

        Sat_Data ParseMissionControlData(std::string &response)
        {
            response = response.substr(response.find('{'));

            int actual_arguments = sscanf(
                response.c_str(), kResponseBodyFormat,
                &sat_data_.id,
                &sat_data_.heartbeat_count,
                &sat_data_.payload_deployed,
                &sat_data_.flight_mode,
                &sat_data_.gps_data.latitude, &sat_data_.gps_data.longitude,
                &sat_data_.barometer_data.altitude, &sat_data_.barometer_data.temperature,
                &sat_data_.imu_data.acceleration_x, &sat_data_.imu_data.acceleration_y, &sat_data_.imu_data.acceleration_z,
                &sat_data_.imu_data.gyro_x, &sat_data_.imu_data.gyro_y, &sat_data_.imu_data.gyro_z,
                &sat_data_.imu_data.pitch, &sat_data_.imu_data.pitch);

            if (actual_arguments != kExpectedNumberOfArguments)
            {
                printf("Received %d arguments, expected %d", actual_arguments, kExpectedNumberOfArguments);
            }
            return sat_data_;
        }

    private:
        Sat_Data sat_data_;
        const int kExpectedNumberOfArguments = 18;
    };

}