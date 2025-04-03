#ifndef _TLE5012B1000_H__
#define _TLE5012B1000_H__
/* Include */

/* value */
extern SPI_HandleTypeDef hspi2;
/* Mapping */
#define SPI_tle5012 hspi2
#define GPIO_TLE5012_x	TLE5012B_CS_GPIO_Port
#define GPIO_TLE5012_PIN TLE5012B_CS_Pin
#define GPIO_TLE5012_PIN2 TLE5012B_CS_Pin2
#define SPI_CS_ENABLE  HAL_GPIO_WritePin(GPIO_TLE5012_x, GPIO_TLE5012_PIN, GPIO_PIN_RESET)       
#define SPI_CS_DISABLE HAL_GPIO_WritePin(GPIO_TLE5012_x, GPIO_TLE5012_PIN, GPIO_PIN_SET)
#define SPI_CS_ENABLE2  HAL_GPIO_WritePin(GPIO_TLE5012_x, GPIO_TLE5012_PIN2, GPIO_PIN_RESET)
#define SPI_CS_DISABLE2 HAL_GPIO_WritePin(GPIO_TLE5012_x, GPIO_TLE5012_PIN2, GPIO_PIN_SET)
#define DEFAULT_TUPD 42.7f				//us  ����Ԥ�⹦��
#define ANG_RATIO	0.010986f				//360/2^15
#define SPD_RATIO	10986.328f				//360*10^6/2^15
/* SPI command for TLE5012 */
#define READ_STATUS					0x8001			//8000    1_0000_0_000000_0000
#define READ_ANGLE_VALUE		0x8021			//8020		1_0000_0_000010_0000
#define READ_SPEED_VALUE		0x8031			//8030		1_0000_0_000011_0000
#define READ_RECOL_VALUE		0x8041			//8030		1_0000_0_000100_0000
#define	READ_MOD_3_VALUE		0xD091			//8030		1_1010_0_001001_0000
#define WRITE_ACSTAT_VALUE  0x0011			//0010		0_0000_0_000001_0000		
 
#define WRITE_MOD1_VALUE		0x5060							//0_1010_0_000110_0001
#define MOD1_VALUE	0x0001
 
#define WRITE_MOD2_VALUE		0x5080							//0_1010_0_001000_0001
#define MOD2_VALUE	0x0801
 
#define WRITE_MOD3_VALUE		0x5091							//0_1010_0_001001_0000
#define MOD3_VALUE	0x0000
 
#define WRITE_MOD4_VALUE		0x50E0							//0_1010_0_001110_0001
#define MOD4_VALUE	0x0098				//9bit 512
 
#define WRITE_IFAB_VALUE		0x50B1
#define IFAB_VALUE 0x000D
/* Functionality mode */
#define REFERESH_ANGLE		0
/* Functionas*/
void tle5012_Init(void);
void tle5012_Rset(void);
void tle5012_Calibrate0(void);
float tle5012_ReadAngle(void);
float tle5012_ReadSpeed(void);
int16_t tle5012_ReadRevol(uint8_t Dir);
uint16_t TlE5012W_Reg(uint16_t Reg_CMD, uint16_t Reg_Data);

#endif
