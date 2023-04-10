#include <HardwareSerial.h>
#include <vector>
#include <string>
#include <map>
#define GPSSerial Serial7
/*
    PA1616S - GPS Module:
        Public functions:
        initialize() - begins GPS serial port
        update() - monitor for GPS coordinate changes
        getLongitude() - gets longitude in GPS coordinates
        getLatitude() - gets latitude in GPS coordinates
*/

/**
 * @brief GPS Module
 *
 * Public functions:
 * initialize() - begins GPS serial port
 * update() - monitor for GPS coordinate changes
 * getLongitude() - gets longitude in GPS coordinates
 * getLatitude() - gets latitude in GPS coordinates
 *
 */
namespace CanSat
{
    class PA1616S
    {
    private:
        std::vector<std::string> GPGGAFormat = {"code", "time_stamp", "latitude", "latitude_direction", "longitude", "longitude_direction", "quality", "satellite_count", "hdop", "altitude", "altitude_units", "geoid_separation", "geoid_separation_units", "seconds_since_update", "reference_station", "checksum"};
        std::map<std::string, std::string> splitData;

        boolean newData = false;
        static const int MAX_BUFFER = 250;
        char receivedCharacters[MAX_BUFFER]; // array holder for incoming chars

        std::map<std::string, std::string> split(std::string data)
        {
            size_t position = data.find(",");
            std::string term;
            int termCount = 0;
            while (position != -1)
            {
                if (position == 0)
                    term = "null";
                else
                    term = data.substr(0, position);
                splitData[GPGGAFormat[termCount]] = term;
                data = data.substr(position + 1);
                position = data.find(",");
                termCount++;
            }

            return splitData;
        }

        float translate_latitude(std::string nmeaLatitude, std::string direction)
        {
            // rawLatitude gives data in degree/minutes, as ddmm.mmmm, needs to be converted to GPS coordinates
            if (nmeaLatitude.compare("") == 0)
                return 0.00;

            std::string stringDegrees = nmeaLatitude.substr(0, 2);
            int degrees = atoi(stringDegrees.c_str());
            const char *stringMinutes = nmeaLatitude.substr(2).c_str();
            float minutes = atof(stringMinutes);
            float gpsLatitude = degrees + (minutes / 60);
            int directionMultiplier = (direction == "N") ? 1 : -1;

            return gpsLatitude * directionMultiplier;
        }

        float translate_longitude(std::string nmeaLongitude, std::string direction)
        {
            if (nmeaLongitude.compare("") == 0)
                return 0.00;

            std::string stringDegrees = nmeaLongitude.substr(0, 3);
            int degrees = atoi(stringDegrees.c_str());
            const char *stringMinutes = nmeaLongitude.substr(3).c_str();
            float minutes = atof(stringMinutes);
            float gpsLongitude = degrees + (minutes / 60);
            int directionMultiplier = (direction == "E") ? 1 : -1;

            return gpsLongitude * directionMultiplier;
        }

        void readData()
        {
            static byte i = 0;
            char currentCharacter;
            int feed_length = 0;

            while (GPSSerial.available() > 0 && newData == false)
            { // here is what breaks stuff
                currentCharacter = GPSSerial.read();
                if (currentCharacter != '\n')
                {
                    receivedCharacters[i] = currentCharacter;
                    i++;
                    if (i >= MAX_BUFFER)
                        i = MAX_BUFFER - 1;
                }
                else
                {
                    receivedCharacters[i] = '\0';
                    i = 0;
                    newData = true;
                }
            }
        }

    public:
        PA1616S()
        {
        }
        void Initialize()
        {
            GPSSerial.begin(9600);
        };

        void Update()
        {
            std::string data;
            readData();
            if (newData == true)
            {
                data = receivedCharacters;
            }

            if (data.length() >= 6 && data.substr(0, 6) == "$GPGGA")
            {
                splitData = split(receivedCharacters);
            }
        }

        float GetLongitude()
        {
            return translate_longitude(splitData["longitude"], splitData["longitude_direction"]);
        }

        float GetLatitude()
        {
            return translate_latitude(splitData["latitude"], splitData["latitude_direction"]);
        }

        float GetAltitude()
        {
            return atof(splitData["altitude"].c_str());
        }
    };
}