#ifndef __NORFLASH_APP_H
#define __NORFLASH_APP_H



void SaveSampleData(void);
void NorFlash_SaveUpgradePara(void);
int NorFlash_ChkSelf(void);
int NorFlash_WriteApp(uint8_t* buff, uint8_t len);
uint32_t NorFlash_ReadAdcData(char *adcDataTime);
void NorFlash_ReadAdcInfo(int si, int ei, char *buf);
#endif







