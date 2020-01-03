#ifndef __NORFLASH_APP_H
#define __NORFLASH_APP_H

void NorFlash_SaveFirmPara(void);
int NorFlash_ChkSelf(void);
int NorFlash_WriteApp(uint8_t* buff, uint8_t len);

#endif







