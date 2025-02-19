

#include <stdint.h>
#include "main.h"

// RM3100 - I2C ADDRESS
// Can be configured used pin3 and pin28
#define RM3100_ADDRESS_LL 0x20
#define RM3100_ADDRESS_LH 0x21
#define RM3100_ADDRESS_HL 0x22
#define RM3100_ADDRESS_HH 0x23


/*
 * // Here is a general equation for gain taken directly from correspondence with PNI which I use in my Python scripts.
// Gn=(Aval*(0.3671*Cycnt+1.5)/1000)
// (0.3671*cycle count + 1.5)  when divided into the X Y or Z result with no averaging gives the correct value in micro teslas
// Aval/1000 times (0.3671*cycle count + 1.5) when divided into the X Y or Z result gives the correct value in nano teslas.
// Conversely, you can multiply the X Y or Z values by  1000/(Aval*(0.3671*Cycnt+1.5))
// As far as I know, it is an exact gain equation for the RM3100 and works for ANY cycle count .... like 375, 405, 125, etc, etc.  No error prone lookup tables.
 */


// MEMORY MAPPING
/*
 * All register addresses are 7bit (bit mask: 0x80)
 * Ex: For write to register address CCX_MSB send:  0b00000100
 * Ex: For read from register address CCX_MSB send: 0b10000100
 * The MSB bit set to read or write command
 *
Name	Register	R/W		Default		Payload Format		Description
------------------------------------------------------------------------------------------
POLL	0x00		RW		0x00		[7:0]				Polls for a Single Measurement
CMM		0x01		RW		0x00		[7:0]				Initiates Continuous Measurement Mode
CCX		0x04-0x05	RW		0x00-0xC8	Uint16				Cycle Count Register – X Axis | See RM3100 user guide table 3-1
CCY		0x06-0x07	RW		0x00-0xC8	Uint16				Cycle Count Register – Y Axis | See RM3100 user guide table 3-1
CCZ		0x04-0x05	RW		0x00-0xC8	Uint16				Cycle Count Register – Z Axis | See RM3100 user guide table 3-1
TMRC	0x0B 		RW 		0x96		[7:0] 				Sets Continuous Measurement Mode Data Rate
ALLX	0x0C–0x0E	RW		0x000000	Sint24				Alarm Lower Limit – X Axis
ALLX	0x0F–0x11	RW		0x000000	Sint24				Alarm Upper Limit – X Axis
ALLY	0x12–0x14	RW		0x000000	Sint24				Alarm Lower Limit – Y Axis
ALLY	0x15–0x17	RW		0x000000	Sint24				Alarm Upper Limit – Y Axis
ALLZ	0x18–0x1A	RW		0x000000	Sint24				Alarm Lower Limit – Z Axis
ALLZ	0x1B–0x1D	RW		0x000000	Sint24				Alarm Upper Limit – Z Axis
ADLX	0x1E–0x1F	RW		0x0000		SInt16				Alarm Hysteresis Value – X Axis
ADLY 	0x20–0x21	RW		0x0000		SInt16 				Alarm Hysteresis Value – Y Axis
ADLZ	0x22–0x23	RW		0x0000		SInt16				Alarm Hysteresis Value – Z Axis
MX		0xA4–0xA6 	R 		0x000000 	Sint24 				Measurement Results – X Axis
MY		0xA7–0xA9	R		0x000000	Sint24				Measurement Results – Y Axis
MZ 		0xAA–0xAC 	R 		0x000000 	Sint24 				Measurement Results – Z Axis
BIST	0x33		RW		0x00		[7:0]				Built-In Self-Test
STATUS 	0x34 		R 		0x00 		[7:0] 				Status of DRDY
HSHAKE	0x35		RW		0x1B		[7:0]				Handshake Register
REVID 	0x36 		R 		-- 			Unit8 				MagI2C Revision Identification
*/

//RM3100 register addresses
#define RM3100_POLL		0x00
#define RM3100_CMM		0x01 //Continuous measurement mode register for write

//CCX, CCY and CCZ values can be set only 0x32(50D), 0x64(100), or 0xC8(200)
#define RM3100_CCX_MSB	0x04 //CCX_MSB address | Default value 0x00 | Read Adsress: 0x84
#define RM3100_CCX_LSB	0x05 //CCX_LSB address | Default value 0xC8 | Read Address: 0x85

#define RM3100_CCY_MSB	0x06 //CCY_MSB address | Default value 0x00 | Read Adsress: 0x86
#define RM3100_CCY_LSB	0x07 //CCY_LSB address | Default value 0xC8 | Read Adsress: 0x87

#define RM3100_CCZ_MSB	0x08 //CCZ_MSB address | Default value 0x00 | Read Adsress: 0x88
#define RM3100_CCZ_LSB	0x09 //CCZ_LSB address | Default value 0xC8 | Read Adsress: 0x89

#define RM3100_CMM		0x01 //Initiate Continuous Measurement Mode Address

#define RM3100_MX_2		0xA4 //X Axis measurement MSB address
#define RM3100_MX_1		0xA5
#define RM3100_MX_0		0xA6 //X Axis measurement LSB address

#define RM3100_MY_2		0xA7 //Y Axis measurement MSB address
#define RM3100_MY_1		0xA8
#define RM3100_MY_0		0xA9 //Y Axis measurement LSB address

#define RM3100_MZ_2		0xAA //Z Axis measurement MSB address
#define RM3100_MZ_1		0xAB
#define RM3100_MZ_0		0xAC //Z Axis measurement LSB address

#define RM3100_TMRC		0x0B //Update rate address
#define RM3100_Alarm	0x0C //Start 0x0C and end to 0x1D. Total byte number is 18.

//CCX, CCY and CCZ Cycle count values
#define CC_050 	0x32 //50 cycle count
#define CC_100	0x64 //100 cycle count
#define CC_200	0xC8 //200 cycle count

//Initate continuous mode register parameters. Alarm bits for only read. That represent measurement is exceed the alarm limits.
//When reading initiate continuous mode register measurement is stop.
// Initiate_Continuous_Mode_register=(Read_Z_Axis<<6)|(Read_Y_Axis<<5)|(Read_X_Axis<<4)|(DRDM<<2)|Continuous_Mode
#define Continuous_Mode_On 	0x01
#define Continuous_Mode_Off	0x00
#define Read_X_Axis_On		0x01
#define Read_X_Axis_Off		0x00
#define Read_Y_Axis_On		0x01
#define Read_Y_Axis_Off		0x00
#define Read_Z_Axis_On		0x01
#define Read_Z_Axis_Off		0x00
#define DRDM				0x02


//Update rate register values
#define Rate_600Hz		0x92 //1.7ms
#define Rate_300Hz		0x93 //3ms
#define Rate_150Hz		0x94 //7ms
#define Rate_75Hz		0x95 //13ms
#define Rate_37Hz		0x96 //27ms
#define Rate_18Hz		0x97 //55ms
#define Rate_9Hz		0x98 //110ms
#define Rate_4P5Hz		0x99 //220ms
#define Rate_2P3Hz		0x9A //440ms
#define Rate_1P2Hz		0x9B //800ms
#define Rate_0P6Hz		0x9C //1.6s
#define Rate_0P3Hz		0x9D //3.3s
#define Rate_0P15Hz		0x9E //6.7s
#define Rate_0P075Hz	0x9F //13s


typedef struct
{
	uint8_t TMRC;
	uint8_t Continuous_Measurement_Value;
	uint8_t Cycle_Count_Value;
}RM3100_;

/*
 * 1. Set cycle count register
 * 	•Start with SSN set HIGH, then set SSN to LOW.
 *	•Send 04H (this is the Write Command Byte to address the MSB for the X axis)
 *	•Send 0 (value for the MSB for the X axis)
 *	•Send 64H (value for the LSB for the X axis - pointer automatically increments)
 *	•Send 0 (value for the MSB for the Y axis - pointer automatically increments)
 *	•Send 64H (value for the LSB for the Y axis - pointer automatically increments)
 *	•Send 0 (value for the MSB for the Z axis - pointer automatically increments)
 *	•Send 64H (value for the LSB for the Z axis - pointer automatically increments)
 *	•Set SSN to HIGH
 * 2. Making reading mode Continuous Mode Register
 * 	• Set 3 axis measurement
 * 	• DRDM bits = 0x02
 * 3. Set Update Rate
 * 4. Read the measurements
 * 5. Gn=(Aval*(0.3671*Cycnt+1.5)/1000) Gain and scaled ut
 */


uint8_t Set_CMM_Register(uint8_t Z_Axis_Read, uint8_t Y_Axis_Read, uint8_t X_Axis_Read, uint8_t DRDM_Value, uint8_t CMM_State);
int32_t Convert_Measurement_to_Int24(uint8_t MSB, uint8_t MID, uint8_t LSB);
