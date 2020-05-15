#ifndef __SI5351_H
#define __SI5351_H

#define SI_CLK0_CONTROL	16			// Register definitions
#define SI_CLK1_CONTROL	17
#define SI_CLK2_CONTROL	18
#define SI_SYNTH_PLL_A	26
#define SI_SYNTH_PLL_B	34
#define SI_SYNTH_MS_0		42
#define SI_SYNTH_MS_1		50
#define SI_SYNTH_MS_2		58
#define SI_PLL_RESET		177

#define SI_R_DIV_1		0x0			// R-division ratio definitions
#define SI_R_DIV_2		0x10
#define SI_R_DIV_4		0x20
#define SI_R_DIV_8		0x30
#define SI_R_DIV_16		0x40
#define SI_R_DIV_32		0x50
#define SI_R_DIV_64		0x60
#define SI_R_DIV_128	0x70

#define SI_CLK_SRC_PLL_A	0x00
#define SI_CLK_SRC_PLL_B	0x20

#define XTAL_FREQ	25000000	// Crystal frequency   



#endif
