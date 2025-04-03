#include "tle5012b.h"
/************************************************
 * FunctionName : tle5012_Init
 * Description  : SPI��ʼ��
 * Parameters   : none
 * Returns      : none
************************************************/
uint8_t stringo[35];

void tle5012_Init(void)
{
	//MX_SPI3_Init();
}


/************************************************
 * FunctionName : tle5012_ReadAngle
 * Description  : ��ȡ���ԽǶ�ֵ
 * Parameters   : none
 * 
 * Returns      : none
************************************************/
float tle5012_ReadAngle(void)
{
	/*0x4000 = -180			0x3FFF = +179.99*/
//	return (float)( tle5012_ReadValue(READ_ANGLE_VALUE) * ANG_RATIO );
	uint16_t var,res1,res2;
	float angle;

	var = READ_ANGLE_VALUE;
	SPI_CS_ENABLE;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res2), 1, 0xff);
	res1 = res1&0x7FFF;
	SPI_CS_DISABLE;
	angle = (float)(res1*ANG_RATIO);
	//if (angle>0) {angle=(180+angle);}
	//if (angle<0) {angle=180+(180+angle);}
	//if (angle<0) {angle=180+(180+angle);}
	//sprintf_min((char*)stringo,"rx angle: %.2f \r\n",angle);
	//HAL_UART_Transmit(&huart2,stringo,sizeof(stringo),0xff);
	return(angle);
}



void tle5012_ReadAngle2(float* angle)
{
	/*0x4000 = -180			0x3FFF = +179.99*/
//	return (float)( tle5012_ReadValue(READ_ANGLE_VALUE) * ANG_RATIO );
	uint16_t var,res1,res2;
	//float angle;

	var = READ_ANGLE_VALUE;
	SPI_CS_ENABLE2;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res2), 1, 0xff);
	res1 = res1&0x7FFF;
	SPI_CS_DISABLE2;
	*angle = (float)(res1*ANG_RATIO);
	//sprintf_min((char*)stringo,"rx angle: %.2f \r\n",*angle);
	//HAL_UART_Transmit(&huart2,stringo,sizeof(stringo),0xff);
	//return(&angle);
}
 
/************************************************
 * FunctionName : tle5012_ReadSpeed
 * Description  : �ֱ��ʹ��ڴֲڣ�����û�� ����ͨ������MOD1��FIR_MD�ֶ����ı�Tupd��������Ҳûɶ��
 * Parameters   : none
 * Returns      : none
************************************************/
float tle5012_ReadSpeed(void)
{
	uint16_t var,res1;
	float speed;

	var = 0x8021;//READ_SPEED_VALUE;
	SPI_CS_ENABLE;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
//	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res2), 1, 0xff);
//	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res3), 1, 0xff);
	res1 = res1&0x7FFF;
	if(res1&0x4000)			//Ϊ����
	{
		res1 = ~res1;
		res1 &= 0x3FFF;
		res1++;
		speed = -(float)(res1*SPD_RATIO/(2*DEFAULT_TUPD));
	}
	else
	{
		res1 &= 0x3FFF;
		speed = (float)(res1*SPD_RATIO/(2*DEFAULT_TUPD));
	}

	SPI_CS_DISABLE;
	//sprintf_min((char*)stringo,"rx speed: %.2f \r\n",speed);
	//HAL_UART_Transmit(&huart2,stringo,sizeof(stringo),0xff);
	return(speed);
}

float tle5012_ReadSpeed2(void)
{
	uint16_t var,res1;
	float speed;

	var = 0x8021;//READ_SPEED_VALUE;
	SPI_CS_ENABLE2;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
//	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res2), 1, 0xff);
//	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res3), 1, 0xff);
	res1 = res1&0x7FFF;
	if(res1&0x4000)			//Ϊ����
	{
		res1 = ~res1;
		res1 &= 0x3FFF;
		res1++;
		speed = -(float)(res1*SPD_RATIO/(2*DEFAULT_TUPD));
	}
	else
	{
		res1 &= 0x3FFF;
		speed = (float)(res1*SPD_RATIO/(2*DEFAULT_TUPD));
	}

	SPI_CS_DISABLE2;
	//sprintf_min((char*)stringo,"rx speed: %.2f \r\n",speed);
	//HAL_UART_Transmit(&huart2,stringo,sizeof(stringo),0xff);
	return(speed);
}
/************************************************
 * FunctionName : tle5012_ReadRevol
 * Description  : ��ת�٣���ʱ���һ
 * Parameters   : DirΪ0������˳�� DirΪ1���෴
 * Returns      : none
************************************************/
int16_t tle5012_ReadRevol(uint8_t Dir)
{
	uint16_t var,res1;
	int16_t revol;
	var = READ_RECOL_VALUE;
	SPI_CS_ENABLE;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
	res1 = res1&0x1FF;
	if(res1&0x100)			//Ϊ����
	{
		res1 = ~res1;
		res1 &= 0x00FF;
		res1++;
		revol = (int16_t)(0-res1);
	}
	else
	{
		res1 &= 0x00FF;
		revol = (int16_t)res1;
	}
	if(Dir)	revol = (int16_t)(0-revol);
	//sprintf_min((char*)stringo,"rx revol: %d \r\n",revol);
	//HAL_UART_Transmit(&huart2,stringo,sizeof(stringo),0xff);
	SPI_CS_DISABLE;
	return(revol);
}

int16_t tle5012_ReadRevol2(uint8_t Dir)
{
	uint16_t var,res1;
	int16_t revol;
	var = READ_RECOL_VALUE;
	SPI_CS_ENABLE2;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
	res1 = res1&0x1FF;
	if(res1&0x100)			//Ϊ����
	{
		res1 = ~res1;
		res1 &= 0x00FF;
		res1++;
		revol = (int16_t)(0-res1);
	}
	else
	{
		res1 &= 0x00FF;
		revol = (int16_t)res1;
	}
	if(Dir)	revol = (int16_t)(0-revol);
	//sprintf_min((char*)stringo,"rx revol: %d \r\n",revol);
	//HAL_UART_Transmit(&huart2,stringo,sizeof(stringo),0xff);
	SPI_CS_DISABLE2;
	return(revol);
}
/************************************************
 * FunctionName : tle5012_Rset
 * Description  : ��λ
 * Parameters   : DirΪ0������˳�� DirΪ1���෴
 * Returns      : none
************************************************/
//0101 1011 1111 1110       Write:0x5BFF��λӲ��
void tle5012_Rset(void)
{
	uint16_t var,res1;
	var = WRITE_ACSTAT_VALUE;
	SPI_CS_ENABLE;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	var = 0x5BFF;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
	SPI_CS_DISABLE;
}
/************************************************
 * FunctionName : tle5012_Calibrate0
 * Description  : �궨��㣬�ݲ�ʹ��
 * Parameters   : 
 * Returns      : none
************************************************/
//E1000 MOD3 ��4λΪ0
//uint16_t test1;
//uint16_t test2;
//uint16_t test3;
//uint16_t test4;
//uint16_t test5;
void tle5012_Calibrate0()
{
	uint16_t var,res1,res2,res3,res4,res5;
	var = READ_ANGLE_VALUE;
	SPI_CS_ENABLE;
	/*����ǰλ��ֵ����������12λ*/
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res1), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res2), 1, 0xff);
	res1 &= 0x7FFF;
	res1 = res1>>3;	
	SPI_CS_DISABLE;
	/*�ر�CRCУ��*/
	var = WRITE_ACSTAT_VALUE;
	SPI_CS_ENABLE;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	var = 0x5AF6;					//�ر�CRCУ�飬����д0x08-0x0F��ҪдCRCֵ�������鷳
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);	
	SPI_CS_DISABLE;
	/*��ȡBASEֵ�����ֵ*/
	var = READ_MOD_3_VALUE;
	SPI_CS_ENABLE;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res3), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res4), 1, 0xff);
	SPI_CS_DISABLE;
	res3 = res3>>4;
	res3 -= res1;
	/*����ֵ�ĵ�12λ��д��BASE�Ĵ���*/
	var = WRITE_MOD3_VALUE;
	SPI_CS_ENABLE;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	var = res3<<4;
	HAL_SPI_Transmit(&SPI_tle5012, (uint8_t *)(&var), 1, 0xff);
	HAL_SPI_Receive(&SPI_tle5012, (uint8_t *)(&res5), 1, 0xff);
	SPI_CS_DISABLE;
}





