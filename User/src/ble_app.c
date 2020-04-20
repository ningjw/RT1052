#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"

#define SET_COMMOND_MODE()       GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 0);
#define BLE_STATUS()             GPIO_PinRead(BOARD_BLE_STATUS_GPIO, BOARD_BLE_STATUS_PIN)

#define EVT_OK       (1 << 0)
#define ETV_TIMTOUT  (1 << 1)

extern void LPUART2_init(void);


uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN] = {0};//串口发送缓冲区
uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN] = {0};//串口接收缓冲区

uint8_t g_puart2RxCnt = 0;
uint8_t g_puart2TxCnt = 0;
uint8_t g_puart2StartRx = 0;
uint32_t  g_puart2RxTimeCnt = 0;
uint32_t ble_event = 0;

TaskHandle_t        BLE_TaskHandle = NULL;//蓝牙任务句柄

static char send_str[164] = {0};
ATCfg_t g_at_cfg = {
    .resp_time = 100,//10ms后检测接受到的数据
    .try_times = 2,
    .try_cnt = 0,
};

/***************************************************************************************
  * @brief   发送一个字符串
  * @input   base:选择端口; data:将要发送的数据
  * @return
***************************************************************************************/
void LPUART2_SendString(const char *str)
{
    LPUART_WriteBlocking(LPUART2, (uint8_t *)str, strlen(str));
}


/*****************************************************************
* 功能：发送AT指令
* 输入: send_buf:发送的字符串
		recv_str：期待回令中包含的子字符串
        p_at_cfg：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t AT_SendCmd(const char *cmd, const char *param, const char *recv_str, ATCfg_t *p_at_cfg)
{
    p_at_cfg->try_cnt = 0;

    memset(send_str, sizeof(send_str), 0);
    strcpy(send_str, "AT");
    strcat(send_str, cmd);
    if (NULL != param) {
        strcat(send_str, "=");
        strcat(send_str, param);
    }
    strcat(send_str, "\r\n");

retry:
    g_puart2StartRx = 0;
    g_puart2RxCnt = 0;
    LPUART2_SendString(send_str);//发送AT指令

    if (NULL == recv_str ) {
        return true;
    }
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, p_at_cfg->resp_time);

    //接收到的数据中包含响应的数据
    if(strstr((char *)g_lpuart2RxBuf, recv_str) != NULL) {
        memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;
        return true;
    } else {
        if(p_at_cfg->try_cnt++ > p_at_cfg->try_times) {
            return false;
        }
        goto retry;//重试
    }
}

/***********************************************************************
  * @ 函数名  ： BLE_AppTask
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BLE_AppTask(void)
{
    uint8_t xReturn = pdFALSE;
    PRINTF("BLE Task Create and Running\r\n");
    uint8_t* sendBuf = NULL;

    SET_COMMOND_MODE();

    //设置波特率为230400
    xReturn = AT_SendCmd(BT_BAUD, "230400", BT_BAUD, &g_at_cfg);
    LPUART_SetBaudRate(LPUART2, 230400, LPUART2_CLOCK_SOURCE);

    /* 设置蓝牙名称 */
    xReturn = AT_SendCmd(BT_NAME, DEVICE_BLE_NAME, RESP_OK, &g_at_cfg);
    if( xReturn == true ) {
        g_sys_para.bleLedStatus = BLE_READY;
    }
	/*进入低功耗模式*/
	LPUART2_SendString("AT+LPM=1\r\n");
	vTaskDelay(100);
    /* 进入透传模式 */
    LPUART2_SendString("AT+TPMODE=1\r\n");
    vTaskDelay(100);
    SET_THROUGHPUT_MODE();

    memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
    g_puart2RxCnt = 0;
//	g_sys_para.bleLedStatus = BLE_UPDATE;
    while(1)
    {
        /*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, portMAX_DELAY);
        if ( pdTRUE == xReturn && ble_event == EVT_OK) {

            /* 处理蓝牙数据协议 */
            sendBuf = ParseProtocol(g_lpuart2RxBuf);

//            /* 升级数据包 */
//            if(g_lpuart2RxBuf[0] == 0xE7 && g_lpuart2RxBuf[1] == 0xE7) {
//                /* 串口回复 */
//                LPUART_WriteBlocking(LPUART2, sendBuf, 7);
//				PRINTF("回复:");
//				for(int i = 0;i<7;i++){
//					PRINTF("%02x ",sendBuf[i]);
//				}
//				PRINTF("\n");
//                /* 是否接受完成整个数据包 */
//                if( g_sys_para.firmUpdate == true) {
//                    //将参数存入Nor Flash
//                    NorFlash_SaveFirmPara();
//                    //关闭所有中断,并复位系统
//                    NVIC_SystemReset();
//                }
//            }
//            /* json数据包 */
//            else 
			if( NULL != sendBuf )
            {
                LPUART2_SendString((char *)sendBuf);
                PRINTF("%s",sendBuf);
                free(sendBuf);
                sendBuf = NULL;
            }
        }
        else if(pdTRUE == xReturn && ble_event == ETV_TIMTOUT) { //接受蓝牙数据超时
            g_puart2StartRx = 0;
        }

        //清空接受到的数据
        memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;

        /* 待机条件为1, 接受到蓝牙数据就清0计数器*/
        if(g_sys_para.inactiveCondition == 1) {
            g_sys_para.inactiveCount = 0;
        }

        /* 判断蓝牙连接状态*/
        if(!BLE_STATUS()) { //Disconnected
            g_sys_para.bleLedStatus = BLE_READY;
        } else if(g_sys_para.bleLedStatus != BLE_UPDATE){ 
            g_sys_para.bleLedStatus = BLE_CONNECT;//Connected
        }
    }
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART2_IRQHandler(void)
{
    uint8_t ucTemp;
    /*串口接收到数据*/
    if ((kLPUART_RxDataRegFullFlag) & LPUART_GetStatusFlags(LPUART2))
    {
        /*读取数据*/
        ucTemp = LPUART_ReadByte(LPUART2);
		
		//蓝牙升级固件时是按照16进制来传送的所以需要区分开来
		if(g_sys_para.bleLedStatus == BLE_UPDATE){
			g_puart2RxTimeCnt = 0;
			/* 将接受到的数据保存到数组*/
			g_lpuart2RxBuf[g_puart2RxCnt++] = ucTemp;
			/*固件升级时,每一包的长度都是固定的176个数据*/
			if(g_puart2RxCnt >= FIRM_ONE_PACKE_LEN) {
				PRINTF("\n包id = %d",g_lpuart2RxBuf[2] | (g_lpuart2RxBuf[3]<<8));
				if((g_lpuart2RxBuf[2] | (g_lpuart2RxBuf[3]<<8))== g_sys_para.firmPacksTotal - 1){
					g_sys_para.bleLedStatus = BLE_CONNECT;
				}
				xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
			}
		}else{
			if( ucTemp == '{') {
				g_puart2StartRx = true;
				g_puart2RxTimeCnt = 0;
				g_puart2RxCnt = 0;
			}

			if(g_puart2RxCnt < LPUART2_BUFF_LEN) {
				/* 将接受到的数据保存到数组*/
				g_lpuart2RxBuf[g_puart2RxCnt++] = ucTemp;
			}else{/* 数据超出指定长度, 清0 */
				g_puart2StartRx = false;
				g_puart2RxCnt = 0;
			}
			
			if( ucTemp == '}') {
				/* 接受完成,该标志清0*/
				g_puart2StartRx = false;
				//接受到Android发送的结束采集信号
				if(strstr((char *)g_lpuart2RxBuf, "{\"Id\":12,") != 0) {
					LPUART2_SendString((char *)g_lpuart2RxBuf);
					g_sys_para.sampNumber = 0;//如果此时正在采集数据, 该代码会触发采集完成信号
				} else {
					/*设置接受完成事件 */
					xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
				}
			}
		}
    }
    /* 发生了未知中断, 重启串口2*/
    else
    {
        g_puart2StartRx = 0;
        memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;
        LPUART_Deinit(LPUART2);
        LPUART2_init();
        LPUART_SetBaudRate(LPUART2, 230400, LPUART2_CLOCK_SOURCE);
    }
    __DSB();
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART2_TimeTick(void)
{
	extern volatile uint32_t g_eventTimeMilliseconds;
	g_eventTimeMilliseconds++;
	
    if(g_puart2StartRx)
    {
        g_puart2RxTimeCnt++;
		if(g_sys_para.bleLedStatus == BLE_UPDATE){
			if(g_puart2RxTimeCnt >= 5000){
				g_puart2RxTimeCnt = 0;
				g_sys_para.bleLedStatus = BLE_CONNECT;//蓝牙升级超时
				PRINTF("接受数据超时,退出升级模式");
			}else if(g_puart2RxTimeCnt == 1000){
				PRINTF("\n接受数据超时,当前接受%d个数据", g_puart2RxCnt);
				xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
			}
		}else if(g_puart2RxTimeCnt >= 200) { //接受数据超时
			g_puart2RxTimeCnt = 0;
            xTaskNotify(BLE_TaskHandle, ETV_TIMTOUT, eSetBits);
        }
    }
}



