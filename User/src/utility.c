#include "main.h"


  /**
* @brief  发送一个字符 
* @param  base:选择端口
* @param  data:将要发送的数据
* @retval 无
*/
void Uart_SendByte(LPUART_Type *base, uint8_t data)
{
  LPUART_WriteByte( base, data);
  while (!(base->STAT & LPUART_STAT_TDRE_MASK));
}


  /**
* @brief  发送一个字符串 
* @param  base:选择端口
* @param  data:将要发送的数据
* @retval 无
*/
void Uart_SendString( LPUART_Type *base,  const char *str)
{
  LPUART_WriteBlocking( base, (const uint8_t*)str, strlen(str));
}



/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
uint16_t CRC16(uint8_t *data,uint32_t length)
{
    uint16_t result;
    uint32_t i,j;

    result=0xFFFF;
    if(length!=0)
    {
        for(i=0; i<length; i++)
        {
            result^=(uint16_t)(data[i]);
            for(j=0; j<8; j++)
            {
                if((result&0x0001)==0x0001)
                {
                    result>>=1;
                    result^=0xA001;	//a001
                }
                else result>>=1;
            }
        }
    }
    return result;
}


