/*
	Code is largely based on code by Mario Canistra, Nathan Seidle, and SparkFun.
*/
#pragma once
#include "Arduino.h"
#include <Wire.h>

#define MPL3115A2_ADDRESS 0x60 

#define MPL3115A2_REGISTER_STATUS_TDR	0x02
#define MPL3115A2_REGISTER_STATUS_PDR	0x04

#define STATUS     0x00
#define WHO_AM_I_ADDR   0x0C
#define WHO_AM_I_CONFIRMATION   0xE
#define OUT_P_MSB  0x01
#define OUT_P_CSB  0x02
#define OUT_P_LSB  0x03
#define OUT_T_MSB  0x04
#define OUT_T_LSB  0x05
#define PT_DATA_CFG 0x13
#define BAR_IN_MSB 0x14
#define BAR_IN_LSB 0x15
#define CTRL_REG1  0x26
#define OFF_P      0x2B
#define OFF_T      0x2C
#define OFF_H      0x2D

namespace StratoLink {
  	class MPL3115A2 {
    private:
		struct data{
			float relativeAltitude = 0.0;
			float pressure = 0.0;
			float temperature = 0.0;
		};
		data barometer_data;
		int sda, scl;
      	void toggleOneShot(){
			byte tempSetting = IIC_Read(CTRL_REG1); //Read current settings
			tempSetting &= ~(1<<1); //Clear OST bit
			IIC_Write(CTRL_REG1, tempSetting);

			tempSetting = IIC_Read(CTRL_REG1); //Read current settings to be safe
			tempSetting |= (1<<1); //Set OST bit
			IIC_Write(CTRL_REG1, tempSetting);
	  	}
      
	  	byte IIC_Read(byte regAddr){
		  	// This function reads one byte over IIC
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(regAddr);  // Address of CTRL_REG1
			Wire1.endTransmission(false); // Send data to I2C dev with option for a repeated start. THIS IS NECESSARY and not supported before Arduino V1.0.1!
			Wire1.requestFrom(MPL3115A2_ADDRESS, 1); // Request the data...
			return Wire1.read();
	  	}

      	void IIC_Write(byte regAddr, byte value){
		  	// This function writes one byto over IIC
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(regAddr);
			Wire1.write(value);
			Wire1.endTransmission(true);
	  	}
		
		bool CheckConnection() {
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(WHO_AM_I_ADDR);
			Wire1.endTransmission();
			Wire1.requestFrom(MPL3115A2_ADDRESS, 1);
			byte response = Wire1.read();
			Serial.println(response);
			return (response == WHO_AM_I_CONFIRMATION);
		}

    public:
		float starting_height;
		float starting_altitude;
		bool pluggedIn;

		//MPL(sda, scl, baseAltitude)
		MPL3115A2(int _sda, int _scl){
			sda = _sda;
			scl = _scl;
		}

		void Initialize(float sampleCount){	
			Wire1.begin();
			Wire1.setSDA(sda);
			Wire1.setSCL(scl);
			
			pluggedIn = CheckConnection();
			Serial.println(pluggedIn);
			
			setModeAltimeter();
			setOversampleRate(7);
			enableEventFlags();

			if (pluggedIn) runCalibration(sampleCount);
			else { 
				Serial.println("Barometer not detected, skipping calibration");
				starting_altitude = 65531;
			}
		};

		void Update(){
			barometer_data.relativeAltitude = ReadAltitude();
			barometer_data.temperature = ReadTemperature();
		}

		data GetData(){
			return barometer_data;
		}

		float ReadAltitude() {
			toggleOneShot();

			// if no new data, return previous value;
			if ((IIC_Read(STATUS) & MPL3115A2_REGISTER_STATUS_PDR ) == 0 ) {
				return barometer_data.relativeAltitude;
			}

			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(OUT_P_MSB);
			Wire1.endTransmission(false);
			if (Wire1.requestFrom(MPL3115A2_ADDRESS, 3) != 3) {
				return barometer_data.relativeAltitude;
			}

			byte msb, lsb, csb;
			msb = Wire1.read();
			csb = Wire1.read();
			lsb = Wire1.read();

			float tempcsb = (lsb >> 4) / 16.0;
			float altitude = (float) ( (msb << 8) | csb ) + tempcsb;
			return (altitude - starting_altitude);
		}

		float ReadTemperature(){
			if((IIC_Read(STATUS) & (1<<1)) == 0) toggleOneShot(); //Toggle the OST bit causing the sensor to immediately take another reading

			if ((IIC_Read(STATUS) & MPL3115A2_REGISTER_STATUS_TDR ) == 0 ) {
				return barometer_data.temperature;
			}

			// Read temperature registers
			Wire1.beginTransmission(MPL3115A2_ADDRESS);
			Wire1.write(OUT_T_MSB);  
			Wire1.endTransmission(false);
			if (Wire1.requestFrom(MPL3115A2_ADDRESS, 2) != 2) {
				return barometer_data.temperature;
			}

			byte msb, lsb;
			msb = Wire1.read();
			lsb = Wire1.read();

			toggleOneShot();

			//Negative temperature fix
			word foo = 0;
			bool negSign = false;

			//Check for 2s compliment
			if(msb > 0x7F)
			{
				foo = ~((msb << 8) + lsb) + 1;  //2's complement
				msb = foo >> 8;
				lsb = foo & 0x00F0; 
				negSign = true;
			}

			// The least significant bytes l_altitude and l_temp are 4-bits fractional values, so you must cast the calulation in (float),
			// shift the value over 4 spots to the right and divide by 16 (since there are 16 values in 4-bits). 
			float templsb = (lsb>>4)/16.0; //temp, fraction of a degree
			float temperature = (float)(msb + templsb);
			if (negSign) temperature = 0 - temperature;
			
			return(temperature);
		}
				
		void setModeAltimeter() {
			byte tempSetting = IIC_Read(CTRL_REG1); //Read current settings
			tempSetting |= (1<<7); //Clear ALT bit
			IIC_Write(CTRL_REG1, tempSetting);
		}

		void setOversampleRate(byte sampleRate){
			if(sampleRate > 7) sampleRate = 7; //OS cannot be larger than 0b.0111
			sampleRate <<= 3; //Align it for the CTRL_REG1 register
			
			byte tempSetting = IIC_Read(CTRL_REG1); //Read current settings
			tempSetting &= B11000111; //Clear out old OS bits
			tempSetting |= sampleRate; //Mask in new OS bits
			IIC_Write(CTRL_REG1, tempSetting);
		};

		void enableEventFlags(){  IIC_Write(PT_DATA_CFG, 0x07); } // Enable all three pressure and temp event flags 

		void runCalibration(float sample_count){
			Serial.println("Running Calibration");
			float current_altitude = 0.0;
			float previous_altitude = 0.0;
			float average_altitude = 0.0;

			Serial.print("Starting altitude: ");
			for (int i = 0; i < sample_count; i++){
				while ( previous_altitude == current_altitude || current_altitude == 0 ){ // Wait till sensor is outputting real numbers (not 0.0), then save initial pressure to starting_pressure
					current_altitude = ReadAltitude();
				}
				Serial.print(current_altitude);
				Serial.print(" + ");
				average_altitude += current_altitude;
				previous_altitude = current_altitude;
			}

			average_altitude /= sample_count;
			starting_altitude = average_altitude;
			starting_height = average_altitude;
			Serial.print(" = ");
			Serial.println(starting_altitude);
		};
  	};
}