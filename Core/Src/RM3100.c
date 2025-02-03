/*
 * RM3100.c
 *
 *  Created on: Jan 17, 2025
 *      Author: kaan.sancakdar
 */

#include <RM3100.h>
#include "main.h"


/*
 * Set the Continuous mode register value
 * Z_Axis_Read: Set 0x01 if Z axis read. Set 0x00 if Z axis not read.
 * Y_Axis_Read: Set 0x01 if Y axis read. Set 0x00 if Y axis not read.
 * X_Axis_Read: Set 0x01 if X axis read. Set 0x00 if X axis not read.
 * DRDM_Value: According to RM3100 user guide table 5-3, DRDY pin HIGH after a full measurement complated. That mean is set this value to 0x02.
 * CMM_State: Set continuous mode on or off.
 */
uint8_t Set_CMM_Register(uint8_t Z_Axis_Read, uint8_t Y_Axis_Read, uint8_t X_Axis_Read, uint8_t DRDM_Value, uint8_t CMM_State)
{
	uint8_t data;
	data=(Z_Axis_Read<<6)|(Y_Axis_Read<<5)|(X_Axis_Read<<4)|(DRDM_Value<<2)|CMM_State;
	return data;
}

int32_t Convert_Measurement_to_Int24(uint8_t MSB, uint8_t MID, uint8_t LSB)
{
	int32_t Measurement_Value;

	Measurement_Value = (MSB << 16) | (MID << 8) | LSB;

			if ((MSB & 0x80) > 0)
			{
				Measurement_Value = (Measurement_Value & 0x7FFFFF) * -1;
			}
	return Measurement_Value;

}

int32_t Get_Micro_Tesla(int32_t Measurement_value, uint8_t Cycle_Count)
{
	int32_t microTesla;

	microTesla=(Measurement_value*(0.3671*Cycle_Count+1.5));

	return microTesla;

}
