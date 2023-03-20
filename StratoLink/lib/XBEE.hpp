#pragma once
#include <SoftwareSerial.h>

namespace StratoLink
{
    class XBEE
    {
    private:
        uint8_t RX_pin;
        uint8_t TX_pin;
        SoftwareSerial XBEE_Serial;

    public:
        XBEE(int rx_pin_, int tx_pin_) : RX_pin(rx_pin_), TX_pin(tx_pin_), XBEE_Serial(RX_pin, TX_pin)
        {
        }

        void Initialize(int baudrate_=9600)
        {
            XBEE_Serial.begin(baudrate_);
        }

        String receiveData()
        {
            String message = "";
            if (XBEE_Serial.available())
            {
                message = XBEE_Serial.readString();
            }
            return message;
        }

        void transmitData(String data)
        {
            if (data.length() > 0 && data.length() <= 256)
            {
                XBEE_Serial.write(data.c_str());
            }
            else
            {
                Serial.println("Error: invalid data length");
            }
        }
    };
}
