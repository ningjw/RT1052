#ifndef __NORFLASH_APP_H
#define __NORFLASH_APP_H



void NorFlash_SaveUpgradePara(void);
int NorFlash_ChkSelf(void);

void NroFlash_SaveBatPercent(void);
void NorFlash_AddAdcData(void);
char NorFlash_ReadAdcData(char *adcDataTime);
void NorFlash_ReadAdcInfo(int si, int ei, char *buf);
#endif







