#ifndef __LPM_APP_H
#define __LPM_APP_H

/* �͹��Ĺ���Ĺ���ģʽ���塣
 * ���ѳ���ʱ��ر�> Dsm>����>�ȴ�>���С�
 */
typedef enum _lpm_power_mode
{
    LPM_PowerModeOverRun = 0, /* ��������ģʽ */
    LPM_PowerModeFullRun, /* ��������ģʽ*/
    LPM_PowerModeLowSpeedRun,/* ��������ģʽ */
    LPM_PowerModeLowPowerRun,/* �͹�������ģʽ */
    LPM_PowerModeRunEnd = LPM_PowerModeLowPowerRun,
		/*
			���ģ���ϵͳ�ȴ�ģʽ�£�cpuʱ�ӱ��ſء�
      *�������趼���Ա��ֻ״̬��ʱ���ſ���CCGR���þ�����
      *��û�з���Ȩ��ʱ��DRAM�����Զ�ˢ��ģʽ��
		*/
    LPM_PowerModeSysIdle, /* ϵͳ����ģʽ */
		 /*
			���ģ��ڵ͹��Ŀ���ģʽ�£�����PLL / PFD���رգ�cpu��Դ�رա�
      *ģ��ģ���ڵ͹���ģʽ�����С�
      *���и������趼�ǵ�Դ�ſ�
      *����������Ա��ֵ�Ƶ����
      * DRAM��ˢ�¡�
		 */
    LPM_PowerModeLPIdle, /* �͹��Ŀ���ģʽ */
		 /*
		 ���ģ������˯��ģʽ�£�����PLL / PFD���رգ�XTAL�رգ�cpu��Դ�رա�
      *��32K RTCʱ���⣬����ʱ�Ӿ��ر�
      *���и������趼�ǵ�Դ�ſ�
      *����������ʱ���ſص�
      * DRAM��ˢ�¡�
      *���ʹ��RTOS������DSM�н���systick
		 */
    LPM_PowerModeSuspend, /* ��ͣģʽ */
    LPM_PowerModeSNVS, /*��Դ�ر�ģʽ��ػ�ģʽ */
    LPM_PowerModeEnd = LPM_PowerModeSNVS/*ö�ٽ�β */
} lpm_power_mode_t;

void LPM_LowPowerRun(void);
void LPM_FullSpeedRun(void);


#endif
