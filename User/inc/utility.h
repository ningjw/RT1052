#ifndef __UTILITY_H
#define __UTILITY_H

void Uart_SendByte(LPUART_Type *base, uint8_t data);
uint16_t CRC16(uint8_t *data,uint8_t length);


#endif
