#ifndef __LPUART1__H
#define __LPUART1__H

void Uart_SendString( LPUART_Type *base,  const char *str);
void BOARD_InitDebugConsole(void);

#endif
