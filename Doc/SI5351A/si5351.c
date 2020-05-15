/*****************************************************
This program was produced by the
CodeWizardAVR V2.03.4 Standard
Automatic Program Generator
© Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project :EW16A 
Version :0.2
Date    :2016-10-09
Author  :fanfan 
Company : 
Comments: 


Chip type           : ATmega128
Program type        : Application
Clock frequency     : 8.000000 MHz
Memory model        : Small
External RAM size   : 0
Data Stack size     : 1024
*****************************************************/

#include <mega64.h>
#include <delay.h>
#include <spi.h>
#include <math.h>
#include <stdlib.h> 
#include <stdio.h>
//sfrw ICR1=0X26;
// Alphanumeric LCD Module functions
#asm
   .equ __lcd_port=0x1B ;PORTA
#endasm
#include <lcd.h>

#define RXB8 1
#define TXB8 0
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE 5
#define RXC 7

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

// I2C Bus functions
#asm
   .equ __i2c_port=0x12 ;PORTD
   .equ __sda_bit=6
   .equ __scl_bit=7
#endasm
#include <i2c.h>



 
#define SI_CLK0_CONTROL	16			// Register definitions
#define SI_CLK1_CONTROL	17
#define SI_CLK2_CONTROL	18
#define SI_SYNTH_PLL_A	26
#define SI_SYNTH_PLL_B	34
#define SI_SYNTH_MS_0		42
#define SI_SYNTH_MS_1		50
#define SI_SYNTH_MS_2		58
#define SI_PLL_RESET		177

#define SI_R_DIV_1		0b00000000			// R-division ratio definitions
#define SI_R_DIV_2		0b00010000
#define SI_R_DIV_4		0b00100000
#define SI_R_DIV_8		0b00110000
#define SI_R_DIV_16		0b01000000
#define SI_R_DIV_32		0b01010000
#define SI_R_DIV_64		0b01100000
#define SI_R_DIV_128		0b01110000

#define SI_CLK_SRC_PLL_A	0b00000000
#define SI_CLK_SRC_PLL_B	0b00100000

#define XTAL_FREQ	25000000	// Crystal frequency   

unsigned char ADDRESS=0b11000000;     


void i2cSendRegister(unsigned char reg, unsigned char data)
{
    
    i2c_start();						// ∑¢∆ º–≈∫≈
    i2c_write(ADDRESS);			// ∑¢5351µÿ÷∑
    i2c_write(reg);					   // –¥ºƒ¥Ê∆˜µÿ÷∑  
    i2c_write(data);	
    i2c_stop();						    // ∑¢Õ£÷π–≈∫≈
}


unsigned char i2cReadRegister(unsigned char reg)
{
	unsigned char data;
	i2c_start();						// ∑¢∆ º–≈∫≈
    i2c_write(ADDRESS);			// ∑¢5351µÿ÷∑ 
    i2c_write(reg);		
	i2c_stop();	
     	
    i2c_start();
    i2c_write(ADDRESS|1);
    data=i2c_read(0);
    i2c_stop();
    
	return data;
}



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

    i2cSendRegister(pll + 0, (P3 & 0x0000FF00) >> 8);
    i2cSendRegister(pll + 1, (P3 & 0x000000FF));
    i2cSendRegister(pll + 2, (P1 & 0x00030000) >> 16);
    i2cSendRegister(pll + 3, (P1 & 0x0000FF00) >> 8);
    i2cSendRegister(pll + 4, (P1 & 0x000000FF));
    i2cSendRegister(pll + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
    i2cSendRegister(pll + 6, (P2 & 0x0000FF00) >> 8);
    i2cSendRegister(pll + 7, (P2 & 0x000000FF));
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

	i2cSendRegister(synth + 0,   (P3 & 0x0000FF00) >> 8);
	i2cSendRegister(synth + 1,   (P3 & 0x000000FF));
	i2cSendRegister(synth + 2,   ((P1 & 0x00030000) >> 16) | rDiv);
	i2cSendRegister(synth + 3,   (P1 & 0x0000FF00) >> 8);
	i2cSendRegister(synth + 4,   (P1 & 0x000000FF));
	i2cSendRegister(synth + 5,   ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
	i2cSendRegister(synth + 6,   (P2 & 0x0000FF00) >> 8);
	i2cSendRegister(synth + 7,   (P2 & 0x000000FF));
}

//
// Switches off Si5351a output
// Example: si5351aOutputOff(SI_CLK0_CONTROL);
// will switch off output CLK0
//
void si5351aOutputOff(unsigned char clk)
{
	i2cSendRegister(clk, 0x80);		// Refer to SiLabs AN619 to see bit values - 0x80 turns off the output stage
}

// 
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
	i2cSendRegister(SI_PLL_RESET, 0xA0);	
									// Finally switch on the CLK0 output (0x4F)
									// and set the MultiSynth0 input to be PLL A
	i2cSendRegister(SI_CLK0_CONTROL, 0x4F | SI_CLK_SRC_PLL_A);
}





unsigned char  beep_flag=0; 
void beep(void)                         //   beep√˘Ω–
{
 PORTG|=0x01;    //   PORTG.0=1;
 beep_flag=4;     
// PORTG&=0xfe; //   PORTG.0=0;
};







// Standard Input/Output functions


// Timer 0 output compare interrupt service routine
interrupt [TIM0_COMP] void timer0_comp_isr(void)            //10 ms    º¸≈Ã…®√Ë∂® ±£¨  ∞¥º¸…˘“Ù ∂® ±
{                                                      
// Place your code here
  if(beep_flag)      //BEEP
  {
   beep_flag--;
  }
  else
  {
   PORTG&=0xfe;    //   PORTG.0=0;
  }
}

// Timer1 output compare A interrupt service routine
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
// Place your code here
  
}



// Declare your global variables here

void main(void)
{
// Declare your local variables here

// Input/Output Ports initialization
// Port A initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTA=0x00;
DDRA=0x00;

// Port B initialization
// Func7=Out Func6=Out Func5=Out Func4=In Func3=In Func2=Out Func1=Out Func0=Out 
// State7=0 State6=0 State5=0 State4=T State3=T State2=0 State1=0 State0=0 
PORTB=0x00;
DDRB=0xE7;

PORTB.0=1;
PORTB.6=1;
PORTB.5=1;

// Port C initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=In Func2=In Func1=In Func0=In 
// State7=0 State6=0 State5=0 State4=0 State3=T State2=T State1=T State0=T 
PORTC=0x00;
DDRC=0xF0;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTD=0x00;
DDRD=0x31;

//DDRD.3=1;
//DDRD.4=1;
//DDRD.5=1;

// Port E initialization
// Func7=In Func6=In Func5=In Func4=In Func3=Out Func2=Out Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=0 State2=0 State1=T State0=T 
PORTE=0x00;
DDRE=0x0C;

// Port F initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTF=0x00;
DDRF=0x00;

// Port G initialization
// Func4=In Func3=In Func2=In Func1=Out Func0=Out 
// State4=T State3=T State2=T State1=0 State0=0 
PORTG=0x00;
DDRG=0x03;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 7.813 kHz
// Mode: CTC top=OCR0
// OC0 output: Disconnected
ASSR=0x00;
TCCR0=0x0F;
TCNT0=0x00;
OCR0=0x4E;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 8000.000 kHz
// Mode: CTC top=OCR1A
// OC1A output: Discon.
// OC1B output: Discon.
// OC1C output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: On
// Compare B Match Interrupt: Off
// Compare C Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x09;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x1F;
OCR1AL=0x40;
OCR1BH=0x00;
OCR1BL=0x00;
OCR1CH=0x00;
OCR1CL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2 output: Disconnected
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;

// Timer/Counter 3 initialization
// Clock source: System Clock
// Clock value: Timer3 Stopped
// Mode: Normal top=0xFFFF
// OC3A output: Discon.
// OC3B output: Discon.
// OC3C output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer3 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
// Compare C Match Interrupt: Off
TCCR3A=0x00;
TCCR3B=0x00;
TCNT3H=0x00;
TCNT3L=0x00;
ICR3H=0x00;
ICR3L=0x00;
OCR3AH=0x00;
OCR3AL=0x00;
OCR3BH=0x00;
OCR3BL=0x00;
OCR3CH=0x00;
OCR3CL=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
// INT3: Off
// INT4: On
// INT4 Mode: Any change
// INT5: On
// INT5 Mode: Any change
// INT6: On
// INT6 Mode: Any change
// INT7: On
// INT7 Mode: Any change
EICRA=0x00;
EICRB=0x5F;       // ÷–∂œÃıº˛£¨4-…œ…˝—ÿ 5-…œ…˝—ÿ 6-»Œ“‚ 7-»Œ“‚      
EIMSK=0xC0;       // ÷–∂œ enable     EIMSK=0xF0; 7 6 5 4    1100-7 6    0xC0; 
EIFR=0xF0;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x12;

ETIMSK=0x00;

// USART0 initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART0 Receiver: On
// USART0 Transmitter: On
// USART0 Mode: Asynchronous
// USART0 Baud Rate: 9600
UCSR0A=0x00;
UCSR0B=0x18;
UCSR0C=0x06;
UBRR0H=0x00;
UBRR0L=0x33;

// USART1 initialization
// USART1 disabled
UCSR1B=0x00;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// ADC initialization
// ADC Clock frequency: 125.000 kHz
// ADC Voltage Reference: Int., cap. on AREF
// Only the 8 most significant bits of
// the AD conversion result are used
ADMUX=0xe2;        //ADC_VREF_TYPE & 0xff;      ƒ⁄≤ø≤Œøº  ADC2 ‰»Î    1110 0010
ADCSRA=0x8e;       // 0x86;   ADC÷–∂œ‘ –Ì        1000 1110

// SPI initialization
// SPI Type: Master
// SPI Clock Rate: 2*2000.000 kHz
// SPI Clock Phase: Cycle Half
// SPI Clock Polarity: Low
// SPI Data Order: MSB First
SPCR=0x54;
SPSR=0x01;      //4M

// TWI initialization
// Bit Rate: 400.000 kHz
//TWBR=0x02;
// Two Wire Bus Slave Address: 0x0
// General Call Recognition: Off
//TWAR=0x00;
// Generate Acknowledge Pulse: Off
// TWI Interrupt: Off
TWCR=0x00;
//TWSR=0x00;
//lcd_init(20);

    
PORTG|=0x01;   //PORTG.0=1;   START BEEP   
delay_ms(120);
PORTG&=0xfe;  //   PORTG.0=0;


while (1)
      {
      // Place your code here
      si5351aSetFrequency(10000000);  
      while (1){}
       
    //    si5351aSetFrequency(5000000);   delay_ms(2000);
      }
}