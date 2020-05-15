#include "main.h"

#define SI5351_ADDR 0xC0

/***************************************************************************************
  * @brief   Write new value to LTC2942 register
  * @input   reg - register number
  * @input   value - new register value
  * @return  
***************************************************************************************/
static void SI5351_WriteReg(uint8_t reg, uint8_t value) {
    lpi2c_master_transfer_t masterXfer = {0};
    uint8_t data = value;
    masterXfer.slaveAddress = SI5351_ADDR;
    masterXfer.direction = kLPI2C_Write;
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data  = &data;;
    masterXfer.dataSize = 1;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;
    
	LPI2C_MasterTransferBlocking(I2C1_MASTER, &masterXfer);
}


/***************************************************************************************
  * @brief   Read LTC2942 register
  * @input   reg - register number
  * @return  register value
***************************************************************************************/
//static uint8_t SI5351_ReadReg(uint8_t reg) {
//	lpi2c_master_transfer_t masterXfer = {0};
//    uint8_t value;
//    
//    masterXfer.slaveAddress = SI5351_ADDR;
//    masterXfer.direction = kLPI2C_Read;
//    masterXfer.subaddress = (uint32_t)reg;
//    masterXfer.subaddressSize = 1;
//    masterXfer.data = &value;
//    masterXfer.dataSize = 1;
//    masterXfer.flags = kLPI2C_TransferDefaultFlag;

//    LPI2C_MasterTransferBlocking(I2C1_MASTER, &masterXfer);
//	return value;
//}

// Set up specified PLL with mult, num and denom
// mult is 15..90
// num is 0..1,048,575 (0xFFFFF)
// denom is 0..1,048,575 (0xFFFFF)
//
void setupPLL(unsigned char pll, unsigned char mult, unsigned long int num, unsigned long int denom)
{
    unsigned long int P1;                    // PLL config register P1
    unsigned long int P2;                    // PLL config register P2
    unsigned long int P3;                    // PLL config register P3

    P1 = (unsigned long int)(128 * ((float)num / (float)denom));
    P1 = (unsigned long int)(128 * (unsigned long int)(mult) + P1 - 512);
    P2 = (unsigned long int)(128 * ((float)num / (float)denom));
    P2 = (unsigned long int)(128 * num - denom * P2);
    P3 = denom;

    SI5351_WriteReg(pll + 0, (P3 & 0x0000FF00) >> 8);
    SI5351_WriteReg(pll + 1, (P3 & 0x000000FF));
    SI5351_WriteReg(pll + 2, (P1 & 0x00030000) >> 16);
    SI5351_WriteReg(pll + 3, (P1 & 0x0000FF00) >> 8);
    SI5351_WriteReg(pll + 4, (P1 & 0x000000FF));
    SI5351_WriteReg(pll + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
    SI5351_WriteReg(pll + 6, (P2 & 0x0000FF00) >> 8);
    SI5351_WriteReg(pll + 7, (P2 & 0x000000FF));
}




//
// Set up MultiSynth with integer divider and R divider
// R divider is the bit value which is OR'ed onto the appropriate register, it is a #define in si5351a.h
//
void setupMultisynth(unsigned char synth, unsigned long int divider, unsigned char rDiv)
{
	unsigned long int P1;					// Synth config register P1
	unsigned long int P2;					// Synth config register P2
	unsigned long int P3;					// Synth config register P3

	P1 = 128 * divider - 512;
	P2 = 0;							// P2 = 0, P3 = 1 forces an integer value for the divider
	P3 = 1;

	SI5351_WriteReg(synth + 0,   (P3 & 0x0000FF00) >> 8);
	SI5351_WriteReg(synth + 1,   (P3 & 0x000000FF));
	SI5351_WriteReg(synth + 2,   ((P1 & 0x00030000) >> 16) | rDiv);
	SI5351_WriteReg(synth + 3,   (P1 & 0x0000FF00) >> 8);
	SI5351_WriteReg(synth + 4,   (P1 & 0x000000FF));
	SI5351_WriteReg(synth + 5,   ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
	SI5351_WriteReg(synth + 6,   (P2 & 0x0000FF00) >> 8);
	SI5351_WriteReg(synth + 7,   (P2 & 0x000000FF));
}


// Set CLK0 output ON and to the specified frequency
// Frequency is in the range 1MHz to 150MHz
// Example: si5351aSetFrequency(10000000);
// will set output CLK0 to 10MHz
//
// This example sets up PLL A
// and MultiSynth 0
// and produces the output on CLK0
//
void si5351aSetFrequency(unsigned long int frequency)
{
	unsigned long int pllFreq;
	unsigned long int xtalFreq = XTAL_FREQ;
	unsigned long int l;
	float f;
	unsigned char mult;
	unsigned long int num;
	unsigned long int denom;
	unsigned long int divider;

	divider = 900000000 / frequency;// Calculate the division ratio. 900,000,000 is the maximum internal 
									// PLL frequency: 900MHz
	if (divider % 2) divider--;		// Ensure an even integer division ratio

	pllFreq = divider * frequency;	// Calculate the pllFrequency: the divider * desired output frequency

	mult = pllFreq / xtalFreq;		// Determine the multiplier to get to the required pllFrequency
	l = pllFreq % xtalFreq;			// It has three parts:
	f = l;							// mult is an integer that must be in the range 15..90
	f *= 1048575;					// num and denom are the fractional parts, the numerator and denominator
	f /= xtalFreq;					// each is 20 bits (range 0..1048575)
	num = f;						// the actual multiplier is  mult + num / denom
	denom = 1048575;				// For simplicity we set the denominator to the maximum 1048575

									// Set up PLL A with the calculated multiplication ratio
	setupPLL(SI_SYNTH_PLL_A, mult, num, denom);
									// Set up MultiSynth divider 0, with the calculated divider. 
									// The final R division stage can divide by a power of two, from 1..128. 
									// reprented by constants SI_R_DIV1 to SI_R_DIV128 (see si5351a.h header file)
									// If you want to output frequencies below 1MHz, you have to use the 
									// final R division stage
	setupMultisynth(SI_SYNTH_MS_0, divider, SI_R_DIV_1);
									// Reset the PLL. This causes a glitch in the output. For small changes to 
									// the parameters, you don't need to reset the PLL, and there is no glitch
	SI5351_WriteReg(SI_PLL_RESET, 0xA0);	
									// Finally switch on the CLK0 output (0x4F)
									// and set the MultiSynth0 input to be PLL A
	SI5351_WriteReg(SI_CLK0_CONTROL, (0x4F | SI_CLK_SRC_PLL_A));
}







