/*************************************************************
 * AL/CC (Pin 5): Alert Output or Charge Complete Input
 *
 *
 *
**************************************************************/
#include "main.h"

const float LTC2942_CHARGE_lsb = 0.085E-3;
const float LTC2942_VOLTAGE_lsb = .3662E-3;
const float LTC2942_TEMPERATURE_lsb = 0.25;
const float LTC2942_FULLSCALE_VOLTAGE = 6;
const float LTC2942_FULLSCALE_TEMPERATURE = 600;

/***************************************************************************************
  * @brief   Write new value to LTC2942 register
  * @input   reg - register number
  * @input   value - new register value
  * @return  
***************************************************************************************/
static void LTC2942_WriteReg(uint8_t reg, uint8_t value) {
    lpi2c_master_transfer_t masterXfer = {0};
    uint8_t data = value;
    masterXfer.slaveAddress = LTC2942_ADDR;
    masterXfer.direction = kLPI2C_Write;
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data  = &data;;
    masterXfer.dataSize = 1;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;
    
	LPI2C_MasterTransferBlocking(LTC2942_I2C_MASTER, &masterXfer);
}


/***************************************************************************************
  * @brief   Read LTC2942 register
  * @input   reg - register number
  * @return  register value
***************************************************************************************/
static uint8_t LTC2942_ReadReg(uint8_t reg) {
	lpi2c_master_transfer_t masterXfer = {0};
    uint8_t value;
    
    masterXfer.slaveAddress = LTC2942_ADDR;
    masterXfer.direction = kLPI2C_Read;
    masterXfer.subaddress = (uint32_t)reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data = &value;
    masterXfer.dataSize = 1;
    masterXfer.flags = kLPI2C_TransferRepeatedStartFlag;

    LPI2C_MasterTransferBlocking(LTC2942_I2C_MASTER, &masterXfer);
	return value;
}


/***************************************************************************************
  * @brief   Read STATUS register,all bits in this register will be cleared after reading
  * @input   
  * @return  value of STATUS register
***************************************************************************************/
inline uint8_t LTC2942_GetStatus(void) {
	return LTC2942_ReadReg(LTC2942_REG_STATUS);
}


/***************************************************************************************
  * @brief   Read CONTROL register
  * @input   
  * @return  value of CONTROL register
***************************************************************************************/
inline uint8_t LTC2942_GetControl(void) {
	return LTC2942_ReadReg(LTC2942_REG_CONTROL);
}


/***************************************************************************************
  * @brief   Read battery voltage, in case of I2C error the return value will be 0x80000
  * @input   
  * @return  voltage in millivolts (value of '4263' represents 4.263V)
***************************************************************************************/
uint32_t LTC2942_GetVoltage(void) {
	uint8_t buf[2];
	uint32_t value = 0; 
    buf[0] = LTC2942_ReadReg(LTC2942_REG_VOL_H);
    buf[1] = LTC2942_ReadReg(LTC2942_REG_VOL_L);
    value  = (buf[0] << 8) | buf[1];
	value *= 6000;
	value /= 65535;
	return value;
}


/***************************************************************************************
  * @brief   Read chip temperature
  * @input   
  * @return  temperature in Celsius (value of '2538' represents 25.38C)
***************************************************************************************/
int32_t LTC2942_GetTemperature(void) {
	uint8_t buf[2];
	uint32_t value = 0; // Initialize with error value in case of I2C timeout

	// Send temperature MSB register address
    buf[0] = LTC2942_ReadReg(LTC2942_REG_TEMP_H);
    buf[1] = LTC2942_ReadReg(LTC2942_REG_TEMP_L);
    
    // Calculate temperature
    value  = ( (buf[0] << 8) | buf[1] ) >> 4;
    value *= 60000;
    value /= 4092;
    // By now the temperature value in Kelvins, convert it to Celsius degrees
    value -= 27315;
    
	return value;
}


/***************************************************************************************
  * @brief   Read accumulated charge value
  * @input   
  * @return  
***************************************************************************************/
uint16_t LTC2942_GetAC(void) {
	uint8_t buf[2];
	uint32_t value = 0x0000; // Initialize with error value in case of I2C timeout
    
    // Send temperature MSB register address
    buf[0] = LTC2942_ReadReg(LTC2942_REG_AC_H);
    buf[1] = LTC2942_ReadReg(LTC2942_REG_AC_L);
    value = buf[1] | (buf[0] << 8);
	return value;
}


/***************************************************************************************
  * @brief   Program accumulated charge value
  * @input   AC - new accumulated charge value
  * @return  
***************************************************************************************/
void LTC2942_SetAC(uint16_t AC) {
	uint8_t reg;

	// Before programming new AC value the analog section must be shut down
	reg = LTC2942_ReadReg(LTC2942_REG_CONTROL);
	LTC2942_WriteReg(LTC2942_REG_CONTROL,reg | LTC2942_CTL_SHUTDOWN);

	// Program new AC value
	LTC2942_WriteReg(LTC2942_REG_AC_H,AC >> 8);
	LTC2942_WriteReg(LTC2942_REG_AC_L,(uint8_t)AC);

	// Restore old CONTROL register value
	LTC2942_WriteReg(LTC2942_REG_CONTROL,reg);
}


/***************************************************************************************
  * @brief   Configure ADC mode
  * @input   mode - new ADC mode (one of LTC2942_ADC_XXX values)
  * @return  
***************************************************************************************/
void LTC2942_SetADCMode(uint8_t mode) {
	uint8_t reg;

	// Read CONTROL register, clear ADC mode bits and configure new value
	reg = LTC2942_ReadReg(LTC2942_REG_CONTROL) & LTC2942_CTL_ADC_MSK;
	LTC2942_WriteReg(LTC2942_REG_CONTROL,reg | mode);
}


/***************************************************************************************
  * @brief   Configure coulomb counter prescaling factor M between 1 and 128
  * @input   psc - new prescaler value (one of LTC2942_PSCM_XXX values)
  * @return  
***************************************************************************************/
void LTC2942_SetPrescaler(uint8_t psc) {
	uint8_t reg;

	// Read CONTROL register, clear prescaler M bits and configure new value
	reg = LTC2942_ReadReg(LTC2942_REG_CONTROL) & LTC2942_CTL_PSCM_MSK;
	LTC2942_WriteReg(LTC2942_REG_CONTROL,reg | psc);
}


/***************************************************************************************
  * @brief   Configure the AL/CC pin
  * @input   mode - new pin configuration (one of LTC2942_ALCC_XXX values)
  * @return  
***************************************************************************************/
void LTC2942_SetALCCMode(uint8_t mode) {
	uint8_t reg;

	// Read CONTROL register, clear AL/CC bits and configure new value
	reg = LTC2942_ReadReg(LTC2942_REG_CONTROL) & LTC2942_CTL_ALCC_MSK;
	LTC2942_WriteReg(LTC2942_REG_CONTROL,reg | mode);
}


/***************************************************************************************
  * @brief   Configure state of the analog section of the chip
  * @input   state - new analog section state (one of LTC2942_AN_XXX values)
  * @return  
***************************************************************************************/
void LTC2942_SetAnalog(uint8_t state) {
	uint8_t reg;

	// Read CONTROL register value
	reg = LTC2942_ReadReg(LTC2942_REG_CONTROL);

	// Set new state of SHUTDOWN bit in CONTROL register B[0]
	if (state == LTC2942_AN_DISABLED) {
		reg |= LTC2942_CTL_SHUTDOWN;
	} else {
		reg &= ~LTC2942_CTL_SHUTDOWN;
	}

	// Write new CONTROL register value
	LTC2942_WriteReg(LTC2942_REG_CONTROL,reg);
}


// The function converts the 16-bit RAW adc_code to Coulombs
float LTC2942_code_to_coulombs(void)
{
	float coulomb_charge;
	coulomb_charge =  1000*(float)(LTC2942_GetAC()*LTC2942_CHARGE_lsb*128*50E-3)/(100*128);
	coulomb_charge = coulomb_charge*3.6f;
	return(coulomb_charge);
}
