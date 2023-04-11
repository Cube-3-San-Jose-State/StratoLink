/*
	Code is largely based on code by Mario Canistra, Nathan Seidle,, and SparkFun.
*/

#include "Arduino.h"
#include <Wire.h>

#define MPL3115A2_ADDRESS 0x60

#define MPL3115A2_REGISTER_STATUS_TDR 0x02
#define MPL3115A2_REGISTER_STATUS_PDR 0x04

#define STATUS 0x00
#define OUT_P_MSB 0x01
#define OUT_P_CSB 0x02
#define OUT_P_LSB 0x03
#define OUT_T_MSB 0x04
#define OUT_T_LSB 0x05
#define PT_DATA_CFG 0x13
#define BAR_IN_MSB 0x14
#define BAR_IN_LSB 0x15
#define CTRL_REG1 0x26
#define OFF_P 0x2B
#define OFF_T 0x2C
#define OFF_H 0x2D

namespace CanSat
{
	class MPL3115A2
	{
	private:
		struct data
		{
			float relativeAltitude = 0.0;
			float pressure = 0.0;
			float temperature = 0.0;
		};
		data barometer_data;
		int sda, scl;
		void toggleOneShot()
		{
			byte tempSetting = IIC_Read(CTRL_REG1); // Read current settings
			tempSetting &= ~(1 << 1);				// Clear OST bit
			IIC_Write(CTRL_REG1, tempSetting);

			tempSetting = IIC_Read(CTRL_REG1); // Read current settings to be safe
			tempSetting |= (1 << 1);		   // Set OST bit
			IIC_Write(CTRL_REG1, tempSetting);
		}

		byte IIC_Read(byte regAddr)
		{
			// This function reads one byte over IIC
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(regAddr);					 // Address of CTRL_REG1
			Wire1.endTransmission(false);			 // Send data to I2C dev with option for a repeated start. THIS IS NECESSARY and not supported before Arduino V1.0.1!
			Wire1.requestFrom(MPL3115A2_ADDRESS, 1); // Request the data...
			return Wire1.read();
		}

		void IIC_Write(byte regAddr, byte value)
		{
			// This function writes one byto over IIC
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(regAddr);
			Wire1.write(value);
			Wire1.endTransmission(true);
		}

	public:
		float starting_height;
		float starting_pressure;

		// MPL(sda, scl, baseAltitude)
		MPL3115A2(int _sda, int _scl)
		{
			sda = _sda;
			scl = _scl;
		}

		void Initialize()
		{
			Wire1.begin();
			Wire1.setSDA(sda);
			Wire1.setSCL(scl);

			setModeStandby();
			setModeBarometer();
			setOversampleRate(7);
			enableEventFlags();
			setModeActive();

			starting_height = 0;
			starting_pressure = 0;

			runCalibration((float)10);
		};

		void Update()
		{
			barometer_data.pressure = ReadPressure();
			if (barometer_data.pressure != 0)
			{
				float height = 44330.77 * (1 - pow((barometer_data.pressure / 101325), .1902632)); // equation is per page 6 on datasheet
				barometer_data.relativeAltitude = height - starting_height;
			}
			barometer_data.temperature = ReadTemperature();
		}

		data GetData()
		{
			return barometer_data;
		}

		float ReadPressure()
		{
			toggleOneShot(); // Toggle the OST bit causing the sensor to immediately take another reading

			// Wait for PDR bit, indicates we have new pressure data
			if ((IIC_Read(STATUS) & MPL3115A2_REGISTER_STATUS_PDR) == 0)
				return barometer_data.pressure;

			// Read pressure registers
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(OUT_P_MSB);
			Wire1.endTransmission(false); // Send data to I2C dev with option for a repeated start. THIS IS NECESSARY and not supported before Arduino V1.0.1!
			if (Wire1.requestFrom(MPL3115A2_ADDRESS, 3) != 3)
			{
				return -999;
			}

			byte msb, csb, lsb;
			msb = Wire1.read();
			csb = Wire1.read();
			lsb = Wire1.read();

			toggleOneShot();

			// Pressure comes back as a left shifted 20 bit number
			long pressure_whole = (long)msb << 16 | (long)csb << 8 | (long)lsb;
			pressure_whole >>= 6; // Pressure is an 18 bit number with 2 bits of decimal. Get rid of decimal portion.

			lsb &= B00110000;						   // Bits 5/4 represent the fractional component
			lsb >>= 4;								   // Get it right aligned
			float pressure_decimal = (float)lsb / 4.0; // Turn it into fraction

			float pressure = (float)pressure_whole + pressure_decimal;
			return (pressure);
		}

		float ReadTemperature()
		{
			if ((IIC_Read(STATUS) & (1 << 1)) == 0)
				toggleOneShot(); // Toggle the OST bit causing the sensor to immediately take another reading
			if ((IIC_Read(STATUS) & MPL3115A2_REGISTER_STATUS_TDR) == 0)
				return barometer_data.temperature; // Returns if no new data

			// Read temperature registers
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(OUT_T_MSB);
			Wire1.endTransmission(false);
			if (Wire1.requestFrom(MPL3115A2_ADDRESS, 2) != 2)
			{
				return -999;
			}

			byte msb, lsb;
			msb = Wire1.read();
			lsb = Wire1.read();

			toggleOneShot();

			// Negative temperature fix
			word foo = 0;
			bool negSign = false;

			// Check for 2s compliment
			if (msb > 0x7F)
			{
				foo = ~((msb << 8) + lsb) + 1; // 2's complement
				msb = foo >> 8;
				lsb = foo & 0x00F0;
				negSign = true;
			}

			// The least significant bytes l_altitude and l_temp are 4-bits fractional values, so you must cast the calulation in (float),
			// shift the value over 4 spots to the right and divide by 16 (since there are 16 values in 4-bits).
			float templsb = (lsb >> 4) / 16.0; // temp, fraction of a degree
			float temperature = (float)(msb + templsb);
			if (negSign)
				temperature = 0 - temperature;

			return (temperature);
		}

		void setModeBarometer()
		{
			byte tempSetting = IIC_Read(CTRL_REG1); // Read current settings
			tempSetting &= ~(1 << 7);				// Clear ALT bit
			IIC_Write(CTRL_REG1, tempSetting);
		}

		void setModeStandby()
		{											// Puts the sensor into Standby mode. Required when changing CTRL1 register
			byte tempSetting = IIC_Read(CTRL_REG1); // Read current settings
			tempSetting &= ~(1 << 0);				// Clear SBYB bit for Standby mode
			IIC_Write(CTRL_REG1, tempSetting);
		}

		void setModeActive()
		{
			byte tempSetting = IIC_Read(CTRL_REG1); // Read current settings
			tempSetting |= (1 << 0);				// Set SBYB bit for Active mode
			IIC_Write(CTRL_REG1, tempSetting);
		}

		void setOversampleRate(byte sampleRate)
		{
			if (sampleRate > 7)
				sampleRate = 7; // OS cannot be larger than 0b.0111
			sampleRate <<= 3;	// Align it for the CTRL_REG1 register

			byte tempSetting = IIC_Read(CTRL_REG1); // Read current settings
			tempSetting &= B11000111;				// Clear out old OS bits
			tempSetting |= sampleRate;				// Mask in new OS bits
			IIC_Write(CTRL_REG1, tempSetting);
		};

		void enableEventFlags() { IIC_Write(PT_DATA_CFG, 0x07); } // Enable all three pressure and temp event flags

		void runCalibration(float sample_count)
		{
			float current_pressure = 0.0;
			float previous_pressure = 0.0;
			float average_pressure = 0.0;

			for (int i = 0; i < sample_count; i++)
			{
				while (previous_pressure == current_pressure || current_pressure == 0.0)
				{ // Wait till sensor is outputting real numbers (not 0.0), then save initial pressure to starting_pressure
					current_pressure = ReadPressure();
				}
				average_pressure += current_pressure;
				previous_pressure = current_pressure;
			}
			average_pressure /= sample_count;
			starting_pressure = average_pressure;
			starting_height = 44330.77 * (1 - pow((starting_pressure / 101325), .1902632));
		};
	};
}