#pragma once

namespace StratoLink
{
    const char kResponseBodyFormat[] =
        "{"
        "\"ID\":\"%c\","
        "\"HRB\":%d,"
        "\"DEP\":%d,"
        "\"FLM\":\"%c\","
        "\"GPS\":[%lf,%lf, %lf],"
        "\"BAR\":[%f,%f],"
        "\"IMU\":[%d,%d,%d,%d,%d,%d,%f,%f]"
        "}";

    const char kGETRequestFormat[] =
        "container?id=%c"
        "&heartbeat_count=%d"
        "&payload_deployed=%d"
        "&flight_mode=%c"
        "&latitude=%lf&longitude=%lf&exactAltitude=%f"
        "&relativeAltitude=%f,&temperature=%f"
        "&acceleration_x=%d,&acceleration_y=%d,&acceleration_z=%d,&gyro_x=%d,&gyro_y=%d,&gyro_z=%d,&pitch=%f,&roll=%f";

    struct GPS_Data
    {
        double latitude;
        double longitude;
        double exactAltitude;
    };

    struct Barometer_Data
    {
        float relativeAltitude;
        float temperature;
    };

    struct IMU_Data
    {
        int acceleration_x;
        int acceleration_y;
        int acceleration_z;
        int gyro_x;
        int gyro_y;
        int gyro_z;
        float pitch;
        float roll;
    };

    struct Sat_Data
    {
        char id;
        int heartbeat_count;
        int payload_deployed;
        char flight_mode;
        GPS_Data gps_data;
        Barometer_Data barometer_data;
        IMU_Data imu_data;
    };

}