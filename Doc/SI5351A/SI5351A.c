/********************************************
����оƬ��STM32F405RGT6��Ƶ168Mhz
����Ƶ�ʣ�HSE=8Mhz  SYSCLK=168Mhz
ģ���ͺţ�STM32���ڳ�ʼ��
ͨѶ��ʽ��
�������ܣ�Si5351ʱ��оƬ
���ߣ�������
��Ȩ��δ������������ֹת��
********************************************/
#include "si5351a.h"
#include "delay.h"
void IICstart()//IIC������ʼ�ź�
{
	  SDA(1);
	  CLK(1);
	  SDA(0);
    delay_us(100); 
	  CLK(0);
    delay_us(100); 	
}
void IICstop()//IIC����ֹͣ�ź�
{
	  CLK(0);
    delay_us(100); 
	  SDA(0);
    delay_us(100); 
	  CLK(1);
    delay_us(100); 
	  SDA(1);	
    delay_us(100); 
}
void IICsend(u8 DATA)//IIC���߷���������
{
		u16 i;
    for(i=0;i<8;i++) //����һ���ֽ����� 
    {     
        CLK(0); 		 //����ʱ���ߣ�׼����ʼ��SDA��ֵ
			
        if((DATA&0x80)==0)  
        {SDA(0);}  
        else  
        {SDA(1);}  
				
        DATA<<=1;  
        delay_us(100);    
        CLK(1);
        delay_us(100);      //�ȴ����豸�����ݽ�����
		}  
    CLK(0);  
    SDA(1);	 				   //�ͷ�������   
    delay_us(100);   
    CLK(1); 
    delay_us(100); 
		while(i<1000){i++;}//�ȴ����豸Ӧ��ACK 
		CLK(0); 					 //�ͷ�ʱ���ߣ�Ϊ�´β�����׼��
}
void IICsendreg(uint8_t reg, uint8_t data)
{
  IICstart();      //��ʼ�ź�
  delay_us(200);  
  IICsend(0xC0);   //�����豸��ַ+д�ź�
  delay_us(200);  
  IICsend(reg);    //�ڲ��Ĵ�����ַ
  delay_us(200);  
  IICsend(data);   //�ڲ��Ĵ�������
  delay_us(200);  
  IICstop(); 
}
void Si5351Init()//��ʼ��Si5351оƬ
{
		GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��
		GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_11;//PB10,11����
		GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;//����ģʽ
		GPIO_Initure.Speed=GPIO_SPEED_HIGH;//GPIO�ٶȸ���
	  HAL_GPIO_Init(GPIOB,&GPIO_Initure);//��ʼ��GPIOB
}
void SetFrequency(uint32_t frequency)//����ʱ��Ƶ��
{
  uint32_t pllFreq;
  uint32_t xtalFreq = XTAL_FREQ;// ����Ƶ��
  uint32_t l;
  float f;
  uint8_t mult;
  uint32_t num;
  uint32_t denom;
  uint32_t divider;
  divider = 900000000 / frequency;	//���໷Ƶ��:900 mhz
  if (divider % 2) divider--;				//ȷ��һ����������������
  pllFreq = divider * frequency;		//����pllFrequency:��Ƶ��*��������Ƶ��
  mult = pllFreq / xtalFreq;				//ȷ�������pllFrequency�ĳ���
  l = pllFreq % xtalFreq;						//������������:
  f = l;														//����һ������,������15 . . 90
  f *= 1048575;											//num�ͷ�ĸ����С������,���Ӻͷ�ĸ
  f /= xtalFreq;										//ÿ20λ(��Χ0 . . 1048575)
  num = f;													//ʵ�ʵĳ����ǳ�+ num /��ĸ��
  denom = 1048575;									//Ϊ��������ǽ���ĸ���1048575
																		//�������໷�뱶��ϵ������
  SetPLLClk(SI_SYNTH_PLL_A, mult, num, denom);
		//����MultiSynth������0�ּ��㡣
		//���R���ѽ׶ο��Գ���2����,��1 . . 128��
		//�ɳ���SI_R_DIV1 reprented SI_R_DIV128(��si5351a��hͷ�ļ�)
		//���������1�׺����������,�����ʹ��
		//���R���ѽ׶�

  SetMultisynth(SI_SYNTH_MS_0,divider,SI_R_DIV_1);
		//�������໷���⽫��������Ĺ��ϡ�����С�ı仯
		//����,����Ҫ��λ���໷,û�й���

  IICsendreg(SI_PLL_RESET,0xA0);	
		//����CLK0���(0 x4f)
		//������MultiSynth0�����໷������
  IICsendreg(SI_CLK0_CONTROL, 0x4F|SI_CLK_SRC_PLL_A);
}
void SetPLLClk(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom)//����PPLʱ��
{
  uint32_t P1;					// PLL config register P1
  uint32_t P2;					// PLL config register P2
  uint32_t P3;					// PLL config register P3

  P1 = (uint32_t)(128 * ((float)num / (float)denom));
  P1 = (uint32_t)(128 * (uint32_t)(mult) + P1 - 512);
  P2 = (uint32_t)(128 * ((float)num / (float)denom));
  P2 = (uint32_t)(128 * num - denom * P2);
  P3 = denom;

  IICsendreg(pll + 0, (P3 & 0x0000FF00) >> 8);
  IICsendreg(pll + 1, (P3 & 0x000000FF));
  IICsendreg(pll + 2, (P1 & 0x00030000) >> 16);
  IICsendreg(pll + 3, (P1 & 0x0000FF00) >> 8);
  IICsendreg(pll + 4, (P1 & 0x000000FF));
  IICsendreg(pll + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
  IICsendreg(pll + 6, (P2 & 0x0000FF00) >> 8);
  IICsendreg(pll + 7, (P2 & 0x000000FF));
}
void SetMultisynth(uint8_t synth,uint32_t divider,uint8_t rDiv)//���ö�synth
{
  uint32_t P1;					// Synth config register P1
  uint32_t P2;					// Synth config register P2
  uint32_t P3;					// Synth config register P3

  P1 = 128 * divider - 512;
  P2 = 0;							// P2 = 0, P3 = 1 forces an integer value for the divider
  P3 = 1;

  IICsendreg(synth + 0,   (P3 & 0x0000FF00) >> 8);
  IICsendreg(synth + 1,   (P3 & 0x000000FF));
  IICsendreg(synth + 2,   ((P1 & 0x00030000) >> 16) | rDiv);
  IICsendreg(synth + 3,   (P1 & 0x0000FF00) >> 8);
  IICsendreg(synth + 4,   (P1 & 0x000000FF));
  IICsendreg(synth + 5,   ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
  IICsendreg(synth + 6,   (P2 & 0x0000FF00) >> 8);
  IICsendreg(synth + 7,   (P2 & 0x000000FF));
}
