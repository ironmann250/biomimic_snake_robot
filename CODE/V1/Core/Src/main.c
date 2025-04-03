/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pid.h"
#include "printf.h"
#include "moving_average.h"
#include "tle5012b.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t  str[] = {"init state                                                                                                                                           \r\n"};
int16_t  counter=0;  
uint16_t pulse_width = 0;
uint8_t string[64];
int16_t revol1,revol2;
uint16_t angle_int;
float speed_,speed_2,rev_rp1,rev_rp2;
uint16_t speed_int;
float angle_;
float angle2;

double rev1=0.0,rev2=0.0;//-0.02*10;
PID_TypeDef motor_1_PID,motor_2_PID;
double measured_angle1, PIDOut1, angle_setpoint1,offset1;
double measured_angle2, PIDOut2, angle_setpoint2,offset2;
uint8_t Rx_data[10]={0,0,0,0,0,0,0,0,0,0};
static uint32_t seed = 1;
float filtered = 0.0;
FilterTypeDef mot1_filterStruct;
FilterTypeDef mot2_filterStruct;
float r_abs(float num);
// Linear Congruential Generator (LCG) parameters
#define LCG_A 1664525
#define LCG_C 1013904223
#define LCG_M 0xFFFFFFFF
#define init_angle_1 250 //40
#define init_angle_2 70 //70
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void badpwm(float dutycycle);
void cw(int speed);
void ccw(int speed);
void srand(uint32_t s);
uint32_t rand(void);
float getRandomFloat(void);
void goto_angle1(double error);
void fast_oscillation1(int speed,float begin , float end);
void fast_oscillation2(int speed,float begin , float end);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define freq 50
float rnd_val=0.0;
double abs_val;
int16_t dutycycle=80;
int16_t T_TOTAL=freq/10;
int16_t T_ON=0;
int16_t count=0;
int16_t pin=0;
int8_t osc_state=-1; //0 go to begin, 1 got end
//SPI_HandleTypeDef hspi2;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */


	//tle5012_Rset();
	//tle5012_Calibrate0();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  srand(HAL_GetTick());
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2);
	//htim3.Instance->CCR1=50;
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	//htim4.Instance->CCR1=50;
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
	//motor stuffs
	HAL_UART_Receive_IT (&huart2, Rx_data, 4);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);

	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
	//htim3.Instance->CCR1=50;
	//PID stuffs
	offset1=2.0/360;
	angle_setpoint1=180;
	angle_setpoint2=180;
	//PID(&motor_1_PID,&rev,&PIDOut,&angle_setpoint, 2, 8, 0, _PID_P_ON_E, _PID_CD_DIRECT); //kinda fast rev
	PID(&motor_1_PID,&rev1,&PIDOut1,&angle_setpoint1, 5.7000, 57.0000, 0.1425, _PID_P_ON_E, _PID_CD_DIRECT); // first angular
	PID_SetMode(&motor_1_PID, _PID_MODE_AUTOMATIC);
	PID_SetSampleTime(&motor_1_PID,4);
	PID_SetOutputLimits(&motor_1_PID, -100, 100);
	PID(&motor_2_PID,&rev2,&PIDOut2,&angle_setpoint2, 5.7000, 57.0000, 0.1425, _PID_P_ON_E, _PID_CD_DIRECT); // first angular
	PID_SetMode(&motor_2_PID, _PID_MODE_AUTOMATIC);
	PID_SetSampleTime(&motor_2_PID,4);
	PID_SetOutputLimits(&motor_2_PID, -100, 100);
	Moving_Average_Init(&mot1_filterStruct);
	Moving_Average_Init(&mot2_filterStruct);
	//set initial angle
	//cw(100);
	//HAL_Delay(500);
	//stop();
	HAL_UART_Transmit(&huart2, str, sizeof(str), 1000);
	//sprintf_min((char*)str,"test\n");//,angle2,revol2);
	HAL_UART_Transmit(&huart2,str,sizeof(str),0xff);
	tle5012_Rset();
	tle5012_Rset();
	tle5012_Rset();
	tle5012_Rset();
	read_sensors();
	read_sensors();
	read_sensors();
	read_sensors();
	read_sensors();
	read_sensors();
	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
	reset_motors();
	tle5012_Rset();
	HAL_Delay(3000);
	angle_setpoint1=init_angle_1;
	angle_setpoint2=init_angle_2;
	//goto_angle1(0.25);
	//goto_angle2(0.25);
	reset_motors();


	//tle5012_Calibrate0();
	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //HAL_UART_Transmit(&huart2,str,sizeof(str),1000);
	  //HAL_UART_Transmit(&huart2,str,sizeof(str),1000);
	  //HAL_UART_Transmit(&huart2, str, sizeof("Hello LGB!\r\n"), 1000);
	  //HAL_Delay(10);
	  //sprintf_min((char*)str,"1: %.2f,%d \n",angle_,revol);//,angle2,revol2);
	  //HAL_UART_Transmit(&huart2,str,sizeof(str),0xff);
	  //sprintf_min((char*)str,"test\n");//,angle2,revol2);
	  //HAL_UART_Transmit(&huart2,str,sizeof(str),0xff);

	  //HAL_UART_Transmit(&huart2, str, sizeof("Hello LGB!\r\n"), 1000);
	  //HAL_UART_Transmit(&huart2, str, sizeof("Hello LGB!\r\n"), 1000);
	  //HAL_UART_Transmit(&huart2, str, sizeof("Hello LGB!\r\n"), 1000);

	  //cw(50);
	  //HAL_Delay(3000);

	  //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
	  //stop();
	  //HAL_Delay(500);
	  //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
	  //ccw(50);
	  //HAL_Delay(5000);
	  //HAL_Delay(500);

	  	  	//angle_setpoint=90;
	  	  	//goto_angle(0.5);
	  	  	//angle_setpoint2=10;
	  	  	//goto_angle2(0.5);
	  	  	//ccw2(100);
	  	  	//stop2();
	  	  	//fast_oscillation2(50,-180,180);
	  	  	/*angle_setpoint=360;
	  	  	goto_angle(0.5);
	  	  	angle_setpoint=0;
	  	  	goto_angle(0.5);
	  	  	angle_setpoint=5.25*360;
	  	  	goto_angle(0.5);
	  	  	angle_setpoint=-5.25*360;
	  	  	goto_angle(0.5);
	  	  	angle_setpoint=0;
	  	  	goto_angle(0.5);*/

	  		/*if (abs_val<1)
	  		{
	  			angle_setpoint=getRandomFloat()*360;
	  			//HAL_Delay(500);


	  		}*/
	  		//=generate_random_float();	  		//HAL_Delay(10);
	  	  	/*
	  		//speed_ = tle5012_ReadSpeed();
	  		//speed_int=(int)speed_*10;
	  		PID_Compute(&motor_1_PID);
	  		//htim3.Instance->CCR1=(int)d_abs(PIDOut);
	  		//PIDOut=100.0;
	  		//PIDOut=-100.0;
	  		if (PIDOut>100.0)
	  		{
	  			PIDOut=100.0;
	  		}
	  		if (PIDOut<-100.0)
	  		{
	  			PIDOut=-100.0;
	  		}
	  		if (PIDOut<0)
	  		{
	  			ccw((int)PIDOut);
	  		}
	  		if	 (PIDOut>0) {
	  			cw((int)PIDOut);
	  		}
	  		if (abs_val<=0.5)
	  		{
	  			stop();
	  			//angle_setpoint=rev;
	  		}*/

	  		//sprintf_min((char*)string,"rx: %d%d%d%d%d%d \r\n",Rx_data[0],Rx_data[1],Rx_data[2],Rx_data[3],Rx_data[4],Rx_data[5]);
	  		//HAL_UART_Transmit(&huart2,string,sizeof(string),0xff);
	  		//sprintf_min((char*)string,"1: %.2f,%d,%.2f  2: %.2f,%d \r\n",angle_,revol,PIDOut,angle2,revol2);
	  read_sensors();
	  goto_angle1(0.5);
	  print_debug();
	  //ccw1(50);
	  		//sprintf_min((char*)str,">target:%.3f\n",angle_setpoint);
	  		//HAL_UART_Transmit(&huart2,str,sizeof(str),0xff);

	  		//HAL_Delay(10);
	  		//HAL_UART_Transmit(&huart2,str,sizeof(str),1000);
//	  		num++;
//
//	  		if(num>50)
//	  		{
//	  			num = 0;
//	  			//tle5012_Rset();
//	  		    //tle5012_Calibrate0();
//	  		}
		//HAL_Delay(500);
		//HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		/*if (counter==0)
		{
			counter=1;
		}
		else
		{
			counter=0;
		}*/
		/*
			T_ON=((dutycycle*T_TOTAL)*10/100);
			htim2.Instance->ARR=100;
		for(int i=0;i<20;i++){
			dutycycle=i;
			HAL_Delay(100);
		}
		for(int i=20;i>0;i--){
			dutycycle=i;
			HAL_Delay(100);
		}*/
		//HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
		 //set to enable
		
		//CLOCKWISE
		//HAL_Delay(500);
		
		/*
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
		htim3.Instance->CCR1=0;
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
		htim4.Instance->CCR1=0;
		HAL_Delay(1000);
		
		//HAL_Delay(500);
		
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
		htim3.Instance->CCR1=100;                                                       ;
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
		htim4.Instance->CCR1=100;                                                       ;
		HAL_Delay(1000);
		)*/
		
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void cw2(int speed)
{
	speed=i_abs(speed);
	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPSIOA,GPIO_PIN_7,GPIO_PIN_SET);
	htim3.Instance->CCR1=speed;
	htim3.Instance->CCR2=0;

	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
	//htim4.Instance->CCR1=0;
	//htim4.Instance->CCR2=speed;
}

void ccw1(int speed)
{
	speed=i_abs(speed);
	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
	htim4.Instance->CCR1=0;
	htim4.Instance->CCR2=speed;
}

void ccw2(int speed)
{
	speed=i_abs(speed);
	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
	htim3.Instance->CCR1=0;
	htim3.Instance->CCR2=speed;

	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
	//htim4.Instance->CCR1=speed;
	//htim4.Instance->CCR2=0;

}

void cw1(int speed)
{
	speed=i_abs(speed);
	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
	htim4.Instance->CCR1=speed;
	htim4.Instance->CCR2=0;

}

void stop2()
{
	htim3.Instance->CCR1=100;
	htim3.Instance->CCR2=100;
	//htim4.Instance->CCR1=0;
}

void stop1()
{
	htim4.Instance->CCR1=100;
	htim4.Instance->CCR2=100;
	//htim4.Instance->CCR1=0;
}
void reset_motors()
{
	htim3.Instance->CCR1=0;
	htim3.Instance->CCR2=0;
	htim4.Instance->CCR1=0;
	htim4.Instance->CCR2=0;
}

void goto_angle1(double error)
{
	read_sensors();
	abs_val=r_abs(rev1-angle_setpoint1);
	while (abs_val>error)
	{
		read_sensors();
		abs_val=r_abs(rev1-angle_setpoint1);
		PID_Compute(&motor_1_PID);
		if (PIDOut1>100.0)
		{
		PIDOut1=100.0;
		}
		if (PIDOut1<-100.0)
		{
		PIDOut1=-100.0;
		}
		if (PIDOut1<0)
		{
		ccw1((int)PIDOut1);
		}
		if (PIDOut1>0) {
		cw1((int)PIDOut1);
		}
		if (abs_val<=error)
		{
			stop1();
			  			//angle_setpoint=rev;
		}
		//sprintf_min((char*)str,"%.2f,%.3f,%.5f,%d,%.2f,%.3f,%.3f\r\n",angle_,rev,angle2,rev2,PIDOut,angle_setpoint,angle_setpoint2,r_abs(rev-angle_setpoint));
		//HAL_UART_Transmit(&huart2,str,sizeof(str),0xff);
	}
	stop1();


}

void goto_angle2(double error)
{

	read_sensors();
	abs_val=r_abs(rev1-angle_setpoint2);
	while (abs_val>error)
	{
		read_sensors();
		abs_val=r_abs(rev1-angle_setpoint2);
		PID_Compute(&motor_2_PID);
		if (PIDOut2>100.0)
		{
		PIDOut2=100.0;
		}
		if (PIDOut2<-100.0)
		{
		PIDOut2=-100.0;
		}
		if (PIDOut2<0)
		{
		cw2((int)PIDOut2);
		}
		if (PIDOut2>0) {
		ccw2((int)PIDOut2);
		}
		if (abs_val<=error)
		{
			stop2();
			  			//angle_setpoint=rev;
		}
		//sprintf_min((char*)str,"%.2f,%.3f,%.5f,%d,%.2f,%.3f,%.3f\r\n",angle_,rev,rev_rp,revol,PIDOut,angle_setpoint,r_abs(rev-angle_setpoint));
		//HAL_UART_Transmit(&huart2,str,sizeof(str),0xff);
	}
	stop2();


}

float r_abs(float num)
{
	if (num<0.0)
		{
			num=-num;
		}
	return num;
}
void print_debug()
{

	sprintf_min((char*)str,"%.2f,%.2f,%.3f,%.3f,%.5f,%.5f,%d,%d,%.2f,%.2f,%.2f,%.2f,%d \r\n",angle_,angle2,rev1,rev2,rev_rp1,rev_rp2,revol1,revol2,PIDOut1,PIDOut2,angle_setpoint1,angle_setpoint2,osc_state);//r_abs(rev1-angle_setpoint1));
	HAL_UART_Transmit(&huart2,str,sizeof(str),0xff);
}
void read_sensors()
{
	angle_ = tle5012_ReadAngle();//Moving_Average_Compute(,&mot1_filterStruct);
	tle5012_ReadAngle2(&angle2);
	//angle2=Moving_Average_Compute(angle2,&mot2_filterStruct);
	revol1 = tle5012_ReadRevol(0);
	revol2 = tle5012_ReadRevol2(0);
	rev1=(revol1+(angle_/360))*360;
	rev2=(revol2+(angle2/360))*360;
	rev_rp1=(revol1+(angle_/360));
	rev_rp2=(revol2+(angle2/360));
	print_debug();
}

void fast_oscillation1(int speed,float begin , float end)
{
	if (osc_state==-1)
	{
		angle_setpoint1=0;
		goto_angle1(0.5);
		osc_state=0;
	}
	if (osc_state==0) //go to begin always ccw cause of state -1
	{
		angle_setpoint1=begin;
		read_sensors();
		if (rev1<begin)
		{
			stop1();
			osc_state=1;
			return;
		}
		else
		{
			ccw1(speed);
			PIDOut1=-speed;
			return;
		}
	}
	if (osc_state==1) //go to end
	{
		angle_setpoint1=end;
		read_sensors();
		if (rev1>end)
			{
				stop1();
				osc_state=0;
				return;
				}
				else
				{
					cw1(speed);
					PIDOut1=speed;
					return;
				}
	}

}

void fast_oscillation2(int speed,float begin , float end)
{
	//begin=-180.0;
	if (osc_state==-1)
	{
		angle_setpoint2=0;
		angle_setpoint1=0;
		goto_angle2(0.5);
		osc_state=0;
		return;
	}
	if (osc_state==0) //go to begin always ccw cause of state -1
	{
		angle_setpoint1=begin;
		read_sensors();
		if (rev2<begin)
		{
			stop2();
			osc_state=1;
			return;
		}
		else
		{
			ccw2(speed);
			PIDOut2=-speed;
			return;
		}
	}
	if (osc_state==1) //go to end
	{
		angle_setpoint1=end;
		read_sensors();
		if (rev2>end)
			{
				stop2();
				osc_state=0;
				return;
				}
				else
				{
					cw2(speed);
					PIDOut2=speed;
					return;
				}
	}

}

void srand(uint32_t s) {
    seed = s;
}

uint32_t rand(void) {
    seed = (LCG_A * seed + LCG_C) & LCG_M;
    return seed;
}

float getRandomFloat(void) {
    uint32_t randomValue = rand();
    // Scale the random number to the range -5 to 5
    float randomFloat = ((float)randomValue / (float)LCG_M) * 10.0f - 5.0f;
    return randomFloat;
}

/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{


  //check 2 numbers
  if (Rx_data[3]-'0'==0)
  {
	  angle_setpoint=(double)(Rx_data[0]-'0')*10+(Rx_data[1]-'0')+offset;
  }
  //check 3 numbers
  else if (Rx_data[3]-'0'==1)
    {
  	  angle_setpoint=(double)(Rx_data[0]-'0')*100+(Rx_data[1]-'0')*10+(Rx_data[2]-'0')+offset;
    }
    //angle_setpoint=d_abs(angle_setpoint);

    sprintf_min((char*)string,"rx: %c %c %c %c %c %c - %.2f\r\n",Rx_data[0],Rx_data[1],Rx_data[2],Rx_data[3],Rx_data[4],Rx_data[5],angle_setpoint);
    HAL_UART_Transmit(&huart2,string,sizeof(string),0xff);
    HAL_UART_Receive_IT(&huart2, Rx_data, 4);

}*/
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM2) {
    static uint8_t fade_direction = 1; // 1: Fade in, 0: Fade out

    if (fade_direction) {
      pulse_width += 10; // Increase pulse width for fade in effect
      if (pulse_width >= 100) {
        fade_direction = 0; // Change direction to fade out
      }
    } else {
      pulse_width -= 10; // Decrease pulse width for fade out effect
      if (pulse_width <= 0) {
        fade_direction = 1; // Change direction to fade in
      }
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse_width);
  }
}*/

void badpwm(float dutycycle)
{		
		int t_on=(int)20.0*(dutycycle/100.0);
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
		pin=1;
		HAL_Delay(t_on);
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
		pin=0;
		HAL_Delay(20-t_on);
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  HAL_Delay(10);
	  HAL_UART_Transmit(&huart2,str,sizeof(str),1000);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
