#include "i2c_eeprom_drv.h"
#include "core_delay.h"
#include "fsl_debug_console.h"

/* ���ڳ�ʱ������ʱ */
#define EEPROM_DELAY_US(x)  CPU_TS_Tmr_Delay_US(x)  

/**
  * @brief ��EEPROMд�����һҳ����
  * @note  ���ñ�������������I2C_EEPROM_WaitStandbyState���еȴ�
  * @param ClientAddr:EEPROM��I2C�豸��ַ(8λ��ַ)
  * @param WriteAddr:д��Ĵ洢��Ԫ�׵�ַ
  * @param pBuffer:������ָ��
  * @param NumByteToWrite:Ҫд���ֽ��������ɳ���EEPROM_PAGE_SIZE�����ֵ
  * @retval  1��������0����
  */
uint32_t I2C_EEPROM_Page_Write( uint8_t ClientAddr,
                                uint8_t WriteAddr,
                                uint8_t* pBuffer,
                                uint8_t NumByteToWrite)
{
    lpi2c_master_transfer_t masterXfer = {0};
    status_t reVal = kStatus_Fail;


    if(NumByteToWrite > EEPROM_PAGE_SIZE)
    {
        return 1;
    }

    /* subAddress = WriteAddr, data = pBuffer �������ӻ�
      ��ʼ�ź�start + �豸��ַslaveaddress(w д����) +
      ���ͻ�������tx data buffer + ֹͣ�ź�stop */

    masterXfer.slaveAddress = (ClientAddr >> 1);
    masterXfer.direction = kLPI2C_Write;
    masterXfer.subaddress = WriteAddr;
    masterXfer.subaddressSize = EEPROM_INER_ADDRESS_SIZE;
    masterXfer.data = pBuffer;
    masterXfer.dataSize = NumByteToWrite;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;

    reVal = LPI2C_MasterTransferBlocking(EEPROM_I2C_MASTER, &masterXfer);

    if (reVal != kStatus_Success)
    {
        return 1;
    }

    return 0;
}

/**
  * @brief ��EEPROMд�벻����������
  * @param ClientAddr:EEPROM��I2C�豸��ַ(8λ��ַ)
  * @param WriteAddr:д��Ĵ洢��Ԫ�׵�ַ
  * @param pBuffer:������ָ��
  * @param NumByteToWrite:Ҫд���ֽ���
  * @retval  ��
  */
void I2C_EEPROM_Buffer_Write( uint8_t ClientAddr,
                              uint8_t WriteAddr,
                              uint8_t* pBuffer,
                              uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;
    /* ����Ҫ������ֽ�������ʼֵΪNumByteToWrite*/
    uint8_t NumByteToWriteRest = NumByteToWrite;
    /* ��������������д���
      1.д����׵�ַ�Ƿ����
      2.���һ��д���Ƿ�պ�д��һҳ */
    Addr = WriteAddr % EEPROM_PAGE_SIZE;
    count = EEPROM_PAGE_SIZE - Addr;

    /* ��NumByteToWrite > count��
       ��һҳд��count���ֽڣ��������ֽ��ٽ��к�������
       ������ (NumByteToWriteRest = NumByteToWrite - count) ���������NumOfPage��NumOfSingle���д���
       ��NumByteToWrite < count��
       ������һҳ���ݣ�ֱ����NumByteToWriteRest = NumByteToWrite���NumOfPage��NumOfSingle���� */
    NumByteToWriteRest = (NumByteToWrite > count) ? (NumByteToWrite - count) : NumByteToWrite;

    /* Ҫ����д���ҳ����������ǰcount�ֽڣ�*/
    NumOfPage =  NumByteToWriteRest / EEPROM_PAGE_SIZE;
    /* ���һҳҪд����ֽ�����������ǰcount�ֽڣ�*/
    NumOfSingle = NumByteToWriteRest % EEPROM_PAGE_SIZE;

    /* NumByteToWrite > countʱ����Ҫ������һҳд��count���ֽ�
       NumByteToWrite < countʱ������д˲��� */
    if(count != 0 && NumByteToWrite > count)
    {
        I2C_EEPROM_Page_Write(ClientAddr, WriteAddr, pBuffer, count);
        I2C_EEPROM_WaitStandbyState(ClientAddr);
        WriteAddr += count;
        pBuffer += count;
    }

    /* ����������� */
    if(NumOfPage == 0 )
    {
        I2C_EEPROM_Page_Write(ClientAddr, WriteAddr, pBuffer, NumOfSingle);
        I2C_EEPROM_WaitStandbyState(ClientAddr);
    }
    else
    {
        /* �������ݴ���һҳ */
        while(NumOfPage--)
        {
            I2C_EEPROM_Page_Write(ClientAddr, WriteAddr, pBuffer, EEPROM_PAGE_SIZE);
            I2C_EEPROM_WaitStandbyState(ClientAddr);
            WriteAddr +=  EEPROM_PAGE_SIZE;
            pBuffer += EEPROM_PAGE_SIZE;
        }
        /* ���һҳ */
        if(NumOfSingle != 0)
        {
            I2C_EEPROM_Page_Write(ClientAddr, WriteAddr, pBuffer, NumOfSingle);
            I2C_EEPROM_WaitStandbyState(ClientAddr);
        }
    }
}

/**
  * @brief ��EEPROM�����ȡһ������
  * @param ClientAddr:EEPROM��I2C�豸��ַ(8λ��ַ)
  * @param ReadAddr:�������ݵ�EEPROM�ĵ�ַ
  * @param pBuffer[out]:��Ŵ�EEPROM��ȡ�����ݵĻ�����ָ��
  * @param NumByteToWrite:Ҫ��EEPROM��ȡ���ֽ���
  * @retval  ��
  */
uint32_t I2C_EEPROM_BufferRead( uint8_t ClientAddr,
                                uint8_t ReadAddr,
                                uint8_t* pBuffer,
                                uint16_t NumByteToRead)
{
    lpi2c_master_transfer_t masterXfer = {0};
    status_t reVal = kStatus_Fail;

    /* subAddress = ReadAddr, data = pBuffer �Դӻ�������
      ��ʼ�ź�start + �豸��ַslaveaddress(w д����) + �ӵ�ַsubAddress +
      �ظ���ʼ�ź�repeated start + �豸��ַslaveaddress(r ������) +
      ���ջ�������rx data buffer + ֹͣ�ź�stop */
    masterXfer.slaveAddress = (ClientAddr >> 1);
    masterXfer.direction = kLPI2C_Read;
    masterXfer.subaddress = (uint32_t)ReadAddr;
    masterXfer.subaddressSize = EEPROM_INER_ADDRESS_SIZE;
    masterXfer.data = pBuffer;
    masterXfer.dataSize = NumByteToRead;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;

    reVal = LPI2C_MasterTransferBlocking(EEPROM_I2C_MASTER, &masterXfer);

    if (reVal != kStatus_Success)
    {
        return 1;
    }

    return 0;
}


/**
  * @brief  ���ڵȴ�EEPROM���ڲ�д��ʱ��
  *         ��I2C_EEPROM_Page_Write��������뱻����
  * @param  ClientAddr:�豸��ַ��8λ��ַ��
  * @retval �ȴ�����Ϊ0���ȴ���ʱΪ1
  */
uint8_t I2C_EEPROM_WaitStandbyState(uint8_t ClientAddr)
{
    status_t lpi2c_status;
    /* �ȴ����� 40*I2CT_LONG_TIMEOUT us����Ϊ��ʱ */
    uint32_t delay_count = I2CT_LONG_TIMEOUT;

    do
    {
        /* �����Ӧ���־���Ա���һ�μ�� */
        LPI2C_MasterClearStatusFlags(EEPROM_I2C_MASTER, kLPI2C_MasterNackDetectFlag);

        /* ��EEPROM����д�����Ѱַ�źţ��Լ���Ƿ���Ӧ */
        lpi2c_status = LPI2C_MasterStart(EEPROM_I2C_MASTER, (ClientAddr >> 1), kLPI2C_Write);

        /* ����ȴ�����30us���Ż������Ӧ���־*/
        EEPROM_DELAY_US(40);

        /* ���LPI2C MSR�Ĵ�����NDF��־������ȷ��delay_countû����0������0��Ϊ��ʱ���˳��� */
    } while(EEPROM_I2C_MASTER->MSR & kLPI2C_MasterNackDetectFlag && delay_count-- );

    /* �����Ӧ���־����ֹ��һ��ͨѶ���� */
    LPI2C_MasterClearStatusFlags(EEPROM_I2C_MASTER, kLPI2C_MasterNackDetectFlag);

    /* ����ֹͣ�źţ���ֹ�´�ͨѶ���� */
    lpi2c_status = LPI2C_MasterStop(EEPROM_I2C_MASTER);
    /* ����ȴ�����10us��ȷ��ֹͣ�źŷ������*/
    EEPROM_DELAY_US(10);

    /* ����ʧ�ܻ�ǰ��ĵȴ���ʱ */
    if(delay_count == 0 || lpi2c_status != kStatus_Success)
    {
        return 1;
    }

    return 0;
}


/*******************************************************************
 * Definition
 *******************************************************************/
#define EEPROM_TEST_NUM           256
#define EEPORM_TEST_START_ADDR    0


/*******************************************************************
 * Variables
 *******************************************************************/
uint8_t EEPROM_Buffer_Write[256];
uint8_t EEPROM_Buffer_Read[256];
/**
  * @brief  I2C(AT24C02)��д����
  * @param  ��
  * @retval ��������0 ������������1
  */
uint8_t EEPROM_Test(void)
{
  uint16_t i;

  PRINTF("д�������");
    
  for ( i=0; i<EEPROM_TEST_NUM; i++ ) //��仺��
  {
    EEPROM_Buffer_Write[i] = i;

    PRINTF("0x%02X ", EEPROM_Buffer_Write[i]);
    if((i+1)%10 == 0 || i == (EEPROM_TEST_NUM-1))    
        PRINTF("\r\n");    
   }
  
  //��I2c_Buf_Write��˳�����������д��EERPOM�� 
  I2C_EEPROM_Buffer_Write(EEPROM_WRITE_ADDRESS_8_BIT,
                             EEPORM_TEST_START_ADDR,
                             EEPROM_Buffer_Write,
                             EEPROM_TEST_NUM);
  
  PRINTF("д�ɹ�");
   
  PRINTF("����������");
  //��EEPROM��������˳�򱣳ֵ�I2c_Buf_Read��
  I2C_EEPROM_BufferRead(EEPROM_READ_ADDRESS_8_BIT,
                           EEPORM_TEST_START_ADDR,
                           EEPROM_Buffer_Read,
                           EEPROM_TEST_NUM); 
   
  //��I2c_Buf_Read�е�����ͨ�����ڴ�ӡ
  for (i=0; i<EEPROM_TEST_NUM; i++)
  { 
    if(EEPROM_Buffer_Read[i] != EEPROM_Buffer_Write[i])
    {
      PRINTF("0x%02X ", EEPROM_Buffer_Read[i]);
      PRINTF("����:I2C EEPROMд������������ݲ�һ��");
      return 1;
    }
    PRINTF("0x%02X ", EEPROM_Buffer_Read[i]);
    if((i+1)%10 == 0 || i == (EEPROM_TEST_NUM-1))    
        PRINTF("\r\n");
    
  }

  PRINTF("I2C(AT24C02)��д���Գɹ�");
  return 0;
}

