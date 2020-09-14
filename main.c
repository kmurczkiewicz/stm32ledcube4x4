#include "main.h"
#include <stdio.h>
#include <stdlib.h>

void setPins();		//Set all pins by global variable
void customSetPins(int postion[4][4]); 		//Set all pins with 2D array of pins state
void customSetLayers(int layerOne, int layerTwo, int layerThree, int layerFour); 	//Custom floor activation
void setLeds(int leds3D[4][4][4], int k); 	//Set pins with 3D array and floor number
void myPattern();	//Function for custom visual effects
void resetPosition();	//Reset 2D array of pins state
void resetLeds();	//Reset 3D array of pins state 
void pixelMove(int x, int y);	//Set only one diode on (x,y,z) position
void pixelMoveWithTail(int x, int y);	//Set one diode on (x,y,z) position, but the last set diode stays on
void pixelReset(int x, int y);	//Reset diode at (x,y) position
void setRandomPins(int homMuch);	//Set rundom number of pins
void VU_Meter(int vu_led[4][4][4]);		//Transfer formatted analog input to setStripe() function
void setStripe(uint16_t number);		//Set "stripe" of diodes

int pinState = 0;   //Global state of pins
int position[4][4]; //2D array of pins state
int leds[4][4][4];  //3D array of pins state
int *lineLeds = leds; // Pointer to 3D array - allow us to refer to 3D array as 1D array


ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

uint16_t adc_val;  //Variable that holds analog input by DMA
uint16_t adc_val_x10[10]; //Test array
uint16_t ledStripes[16][4] = {
		//2D array holding stripes of diodes
		{0,4,8,12},
		{1,5,9,13},
		{2,6,10,14},
		{3,7,11,15},

		{16,20,24,28},
		{17,21,25,29},
		{18,22,26,30},
		{19,23,27,31},

		{32,36,40,44},
		{33,37,41,45},
		{34,38,42,46},
		{35,39,43,47},

		{48,52,56,60},
		{49,53,57,61},
		{50,54,58,62},
		{51,55,59,63}
};


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);

int main(void)
 {
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();

    //Setting timers for PWN
    //D7 - floor 4
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // <-- Set timer 1 on channel 1
    TIM1->CCR1 = 25; // <-- Set 25% from 3,3V (0.8) on this channel

    //D9 - floor 3
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // <-- Set timer 3 on channel 2
    TIM3->CCR2 = 25; // <-- Set 25% from 3,3V (0.8) on this channel

    //D2 - floor 2
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3); // <-- Set timer 1 on channel 3
    TIM1->CCR3 = 25; // <-- Set 25% from 3,3V (0.8) on this channel

    //D8 - floor 1
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // <-- Set timer 1 on channel 2
    TIM1->CCR2 = 25; // <-- Set 25% from 3,3V (0.8) on this channel

  //Turn on reading analog input (DMA)
  HAL_ADC_Start_DMA(&hadc1, &adc_val, 1);

  short f=0;
  while (1)
  {
	  	  VU_Meter(leds);
	  	  f = (f+1)%4;
	  	  setLeds(leds,f);
	  	  resetLeds();
	  	  /*All
			lineLeds[] - pointer to 3D array
			example: lineLeds[0] = left bottom corner diode
			lineLeds[63] = top right corner diode

	  	  //All leds ride 
	  	  for(int i =0; i < 64; i++){
				lineLeds[i] = 1;
				setLeds(leds);
	  	  }

	  	  //VU_Meter();
	  	  //drumKickTest();
	  	  //setPins();
	  	  //myPattern();
	  	  //randomPixelOnButtonPrees();

	  	  //Set floors one by one
	  	  TIM1->CCR1 = 0;
	  	  TIM1->CCR2 = 25;
	  	  HAL_Delay(250);
	  	  TIM1->CCR2 = 0;
	  	  TIM1->CCR3 = 25;
	  	  HAL_Delay(250);
	  	  TIM1->CCR3 = 0;
	  	  TIM3->CCR2 = 25;
	  	  HAL_Delay(250);
	  	  TIM3->CCR2 = 0;
	  	  TIM1->CCR1 = 25;
	  	  HAL_Delay(250);

	  	  1. Set pins with a button click
	  	  if(HAL_GPIO_ReadPin(BUTTON_GPIO_Port,BUTTON_Pin) == 0){
	  			  pinState = 1;
	  			  setPins();
	  	  }

	  	  pinState = 1;
	  	  setPins();
	  	  HAL_Delay(250);
	  	  pinState = 0;
	  	  setPins();
	  	  HAL_Delay(250);
	  	  */
  }

}

void setLeds(int leds3D[4][4][4], int k){
		//Floor
		switch(k){
		case 0:
			customSetLayers(1,0,0,0);
			break;
		case 1:
			customSetLayers(0,1,0,0);
			break;
		case 2:
			customSetLayers(0,0,1,0);
			break;
		case 3:
			customSetLayers(0,0,0,1);
			break;
		}
		//Diode
		  HAL_GPIO_WritePin(D5_GPIO_Port,D5_Pin,leds3D[0][0][k]);
		  HAL_GPIO_WritePin(D15_GPIO_Port,D15_Pin,leds3D[0][1][k]);
		  HAL_GPIO_WritePin(A2_GPIO_Port,A2_Pin,leds3D[0][2][k]);
		  HAL_GPIO_WritePin(D3_GPIO_Port,D3_Pin,leds3D[0][3][k]);

		  HAL_GPIO_WritePin(D14_GPIO_Port,D14_Pin,leds3D[1][0][k]);
		  HAL_GPIO_WritePin(A4_GPIO_Port,A4_Pin,leds3D[1][1][k]);
		  HAL_GPIO_WritePin(A1_GPIO_Port,A1_Pin,leds3D[1][2][k]);
		  HAL_GPIO_WritePin(A5_GPIO_Port,A5_Pin,leds3D[1][3][k]);

		  HAL_GPIO_WritePin(D12_GPIO_Port,D12_Pin,leds3D[2][0][k]);
		  HAL_GPIO_WritePin(D13_GPIO_Port,D13_Pin,leds3D[2][1][k]);
		  HAL_GPIO_WritePin(A0_GPIO_Port,A0_Pin,leds3D[2][2][k]);
		  HAL_GPIO_WritePin(D10_GPIO_Port,D10_Pin,leds3D[2][3][k]);

		  HAL_GPIO_WritePin(D4_GPIO_Port,D4_Pin,leds3D[3][0][k]);
		  HAL_GPIO_WritePin(D6_GPIO_Port,D6_Pin,leds3D[3][1][k]);
		  HAL_GPIO_WritePin(A3_GPIO_Port,A3_Pin,leds3D[3][2][k]);
		  HAL_GPIO_WritePin(D11_GPIO_Port,D11_Pin,leds3D[3][3][k]);
}

void resetLeds(){
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			for(int k = 0; k < 4; k++){
				leds[i][j][k] = 0;
			}
		}
	}
}

void setStripe(uint16_t number){
	if(number > 16)
		number = 16;

	for(int i = 0; i<4; i++){
		for(int j = 0; j < number; j++){
			lineLeds[ledStripes[j][i]] = 1;
		}
	}
}

void VU_Meter(int vu_leds[4][4][4]){
	setStripe(adc_val/90);
}


void setRandomPins(int homMuch){
	resetPosition();
	for(int i = 0; i < homMuch; i++){
		int x = rand() % 4, y =rand() % 4;
		while(position[x][y] == 1){
			x = rand() % 4;
			y =rand() % 4;
		}
		position[x][y] = 1;
	}
}

void randomPixelOnButtonPrees(){
	int layer = rand() % 4, x = rand() % 4, y =rand() % 4;
	  if(HAL_GPIO_ReadPin(BUTTON_GPIO_Port,BUTTON_Pin) == 0){
		  if(layer == 0){
			  customSetLayers(1,0,0,0);
		  }
		  if(layer == 1){
		  	customSetLayers(0,1,0,0);
		  }
		  if(layer == 2){
		  	customSetLayers(0,0,1,0);
		  }
		  if(layer == 3){
		  	customSetLayers(0,0,0,1);
		  }
		  pixelMove(x,y);
		  HAL_Delay(100);
	  }
}

void myPattern(){
	resetPosition();
	customSetLayers(1,0,0,0);

	position[0][0] = 1;
	position[0][3] = 1;
	position[3][0] = 1;
	position[3][3] = 1;

	position[1][1] = 1;
	position[1][2] = 1;
	position[2][1] = 1;
	position[2][2] = 1;
	customSetPins(position);

	//Second floor corners
	HAL_Delay(100);
	customSetLayers(1,1,0,0);
	//Third floor corners
	HAL_Delay(100);
	customSetLayers(1,1,1,0);
	//Fourth floor corners
	HAL_Delay(100);
	customSetLayers(1,1,1,1);

	//Side reset
	position[0][0] = 0;
	customSetPins(position);
	HAL_Delay(100);

	position[0][3] = 0;
	customSetPins(position);
	HAL_Delay(100);

	position[3][0] = 0;
	customSetPins(position);
	HAL_Delay(100);

	position[3][3] = 0;
	customSetPins(position);
	HAL_Delay(100);

	//Floor reset
	customSetLayers(0,1,1,1);
	HAL_Delay(100);
	//Second floor
	HAL_Delay(100);
	customSetLayers(0,0,1,1);
	//Third floor
	HAL_Delay(100);
	customSetLayers(0,0,0,1);


	//Top set
	int i =0, j=0;
	   for(i=0; i<4; i++) {
	      for(j=0;j<4;j++) {
	    	  if((i==1 && j == 1) || (i==1 && j == 2) ||(i==2 && j == 1) || (i==2 && j == 2)){
	    	  }
	    	  else{
				  position[i][j] = 1;
				  customSetPins(position);
				  HAL_Delay(100);
	    	  }
	      }
	 }


for(int i = 0; i <3; i++){
	//Set floors
	customSetLayers(0,0,0,1);
	//Third floor
	HAL_Delay(100);
	customSetLayers(0,0,1,1);
	//Second floor
	HAL_Delay(100);
	customSetLayers(0,1,1,1);
	//First floor
	HAL_Delay(100);
	customSetLayers(1,1,1,1);

	//Reset floors
	customSetLayers(1,1,1,0);
	//Third floor
	HAL_Delay(100);
	customSetLayers(1,1,0,0);
	//Second floor
	HAL_Delay(100);
	customSetLayers(1,0,0,0);
	//First floor
	HAL_Delay(100);
	customSetLayers(0,0,0,0);

	HAL_Delay(200);
}
	pinState = 0;
	setPins();

	for(int i =0; i<5; i++){
	//Explosion
	resetPosition();
	customSetLayers(0,0,1,0);
	//Phase 1
	  position[1][2] = 1;
	  customSetPins(position);
	  HAL_Delay(150);
	//Phase 2
	  customSetLayers(0,1,1,0);
	  position[1][1] = 1;
	  position[2][2] = 1;
	  position[2][1] = 1;
	  customSetPins(position);
	  HAL_Delay(150);
	//Phase 3
	  customSetLayers(1,1,1,1);
	  pinState=1;
	  setPins();
	  HAL_Delay(100);
	  pinState=0;
	  setPins();
	}

	for(int i =0; i<5; i++){
	//Reverse Explosion
	  //Phase 3
	  customSetLayers(1,1,1,1);
	  pinState=1;
	  setPins();
	  HAL_Delay(100);
	  //Phase 2
	  customSetLayers(0,1,1,0);
	  position[1][1] = 1;
	  position[2][2] = 1;
	  position[2][1] = 1;
	  customSetPins(position);
	  HAL_Delay(150);
	  //Phase 1
	  resetPosition();
	  customSetLayers(0,0,1,0);
	  position[1][2] = 1;
	  customSetPins(position);
	  HAL_Delay(150);

	}

	//Moving pixel to front
	pixelMove(2,2);

	pixelMove(3,2);

	pixelMove(3,3);
for(int i = 0; i <3; i++){
	//Pixel pattern////////////////////////////////////////////////
	HAL_Delay(100);
	customSetLayers(0,1,0,0);
	HAL_Delay(100);
	customSetLayers(1,0,0,0);

	pixelMove(3,2);

	pixelMove(3,1);

	pixelMove(3,0);

	//Left top corner
	customSetLayers(0,1,0,0);
	HAL_Delay(100);
	customSetLayers(0,0,1,0);
	HAL_Delay(100);
	customSetLayers(0,0,0,1);
	HAL_Delay(100);

	pixelMove(3,1);

	pixelMove(3,2);

	pixelMove(3,3);

	//Right top corner
	customSetLayers(0,0,1,0);
	HAL_Delay(100);
	customSetLayers(0,1,0,0);
	HAL_Delay(100);

	resetPosition();
	position[3][2] = 1;
	customSetPins(position);
	HAL_Delay(100);

	pixelMove(3,1);

	customSetLayers(0,0,1,0);
	HAL_Delay(100);

	pixelMove(3,2);

	//Pixel pattern////////////////////////////////////////////////
}
	pixelMove(3,3);
	customSetLayers(0,1,0,0);
	customSetLayers(1,0,0,0);

	for(int i=3; i>=0; i--) {
	      for(int j=3;j>=0;j--) {
	    	  pixelMoveWithTail(i,j);
	    }
	 }


	resetPosition();
	position[0][0] = 1;
	customSetPins(position);
	customSetLayers(0,1,0,0);
	HAL_Delay(100);
	customSetLayers(0,0,1,0);
	HAL_Delay(100);
	customSetLayers(0,0,0,1);
	HAL_Delay(100);

	for(int i=0; i<4; i++) {
	      for(int j=0;j<4;j++) {
	    	  pixelMoveWithTail(i,j);
	    }
	 }

	pixelMove(3,3);
	pixelMoveWithTail(3,2);
	pixelMoveWithTail(3,1);
	pixelMoveWithTail(3,0);
	customSetLayers(0,0,1,1);
	pixelMoveWithTail(3,1);
	pixelMoveWithTail(3,2);
	pixelMoveWithTail(3,3);
	customSetLayers(0,1,1,1);
	pixelMoveWithTail(3,2);
	pixelMoveWithTail(3,1);
	pixelMoveWithTail(3,0);
	customSetLayers(1,1,1,1);
	pixelMoveWithTail(3,1);
	pixelMoveWithTail(3,2);
	pixelMoveWithTail(3,3);

	HAL_Delay(100);
	for(int i =0; i <5; i++){
		pixelReset(3,0);
		pixelReset(3,1);

		pixelReset(3,2);
		pixelMoveWithTail(3,0);

		pixelReset(3,3);
		pixelMoveWithTail(3,1);

		pixelMoveWithTail(3,2);

		pixelMoveWithTail(3,3);
	}

	customSetLayers(1,1,1,0);
	HAL_Delay(100);
	customSetLayers(1,1,0,0);
	HAL_Delay(100);
	customSetLayers(1,0,0,0);
	HAL_Delay(100);

	pixelReset(3,0);
	pixelReset(3,1);
	pixelReset(3,2);

	customSetLayers(1,1,0,0);
	HAL_Delay(100);
	customSetLayers(1,1,1,0);
	HAL_Delay(100);
	customSetLayers(1,1,1,1);
	HAL_Delay(100);


	for(int i=0; i<4; i++) {
	      for(int j=0;j<4;j++) {
	    	  pixelMoveWithTail(i,j);
	    }
	 }


	//Reset
	HAL_Delay(2000);
	pinState = 0;
	setPins();
	customSetLayers(0,0,0,0);
	HAL_Delay(100);
}

void pixelMove(int x, int y){
	resetPosition();
	position[x][y] = 1;
	customSetPins(position);
	HAL_Delay(100);
}

void pixelReset(int x, int y){
	position[x][y] = 0;
	customSetPins(position);
	HAL_Delay(100);
}

void pixelMoveWithTail(int x, int y){
	position[x][y] = 1;
	customSetPins(position);
	HAL_Delay(100);
}

void customSetLayers(int layerOne, int layerTwo, int layerThree, int layerFour){
	//Floor 1
	if(layerOne == 0){
		 TIM1->CCR2 = 0;
	}
	else if(layerOne == 1){
		TIM1->CCR2 = 25;
	}

	//Floor 2
	if(layerTwo == 0){
		TIM1->CCR3 =0;
	}
	else if(layerTwo == 1){
		TIM1->CCR3 =25;
	}

	//Floor 3
	if(layerThree == 0){
		TIM3->CCR2 = 0;
	}
	else if(layerThree == 1){
		TIM3->CCR2 = 25;
	}

	//Floor 4
	if(layerFour == 0){
		TIM1->CCR1 = 0;
	}
	else if(layerFour == 1){
		TIM1->CCR1 = 25;
	}
}

void customSetPins(int postition[4][4]){
	  HAL_GPIO_WritePin(D5_GPIO_Port,D5_Pin,postition[0][0]);
	  HAL_GPIO_WritePin(D15_GPIO_Port,D15_Pin,postition[0][1]);
	  HAL_GPIO_WritePin(A2_GPIO_Port,A2_Pin,postition[0][2]);
	  HAL_GPIO_WritePin(D3_GPIO_Port,D3_Pin,postition[0][3]);

	  HAL_GPIO_WritePin(D14_GPIO_Port,D14_Pin,postition[1][0]);
	  HAL_GPIO_WritePin(A4_GPIO_Port,A4_Pin,postition[1][1]);
	  HAL_GPIO_WritePin(A1_GPIO_Port,A1_Pin,postition[1][2]);
	  HAL_GPIO_WritePin(A5_GPIO_Port,A5_Pin,postition[1][3]);

	  HAL_GPIO_WritePin(D12_GPIO_Port,D12_Pin,postition[2][0]);
	  HAL_GPIO_WritePin(D13_GPIO_Port,D13_Pin,postition[2][1]);
	  HAL_GPIO_WritePin(A0_GPIO_Port,A0_Pin,postition[2][2]);
	  HAL_GPIO_WritePin(D10_GPIO_Port,D10_Pin,postition[2][3]);

	  HAL_GPIO_WritePin(D4_GPIO_Port,D4_Pin,postition[3][0]);
	  HAL_GPIO_WritePin(D6_GPIO_Port,D6_Pin,postition[3][1]);
	  HAL_GPIO_WritePin(A3_GPIO_Port,A3_Pin,postition[3][2]);
	  HAL_GPIO_WritePin(D11_GPIO_Port,D11_Pin,postition[3][3]);

}


void resetPosition(){
	int i =0, j=0;
	   for(i=0; i<4; i++) {
	      for(j=0;j<4;j++) {
	    	  position[i][j] = 0;
	      }
	   }
}

void setPins(){
	  //PINS ON RIGHT
	  HAL_GPIO_WritePin(D3_GPIO_Port,D3_Pin,pinState);
	  HAL_GPIO_WritePin(D4_GPIO_Port,D4_Pin,pinState);
	  HAL_GPIO_WritePin(D5_GPIO_Port,D5_Pin,pinState);
	  HAL_GPIO_WritePin(D6_GPIO_Port,D6_Pin,pinState);
	  HAL_GPIO_WritePin(D10_GPIO_Port,D10_Pin,pinState);
	  HAL_GPIO_WritePin(D11_GPIO_Port,D11_Pin,pinState);
	  HAL_GPIO_WritePin(D12_GPIO_Port,D12_Pin,pinState);
	  HAL_GPIO_WritePin(D13_GPIO_Port,D13_Pin,pinState);
	  HAL_GPIO_WritePin(D14_GPIO_Port,D14_Pin,pinState);
	  HAL_GPIO_WritePin(D15_GPIO_Port,D15_Pin,pinState);
	  //PINS (ANALOG) ON LEFT
	  HAL_GPIO_WritePin(A0_GPIO_Port,A0_Pin,pinState);
	  HAL_GPIO_WritePin(A1_GPIO_Port,A1_Pin,pinState);
	  HAL_GPIO_WritePin(A2_GPIO_Port,A2_Pin,pinState);
	  HAL_GPIO_WritePin(A3_GPIO_Port,A3_Pin,pinState);
	  HAL_GPIO_WritePin(A4_GPIO_Port,A4_Pin,pinState);
	  HAL_GPIO_WritePin(A5_GPIO_Port,A5_Pin,pinState);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 4999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim1);

}

static void MX_TIM3_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 4999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 99;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim3);

}

static void MX_DMA_Init(void) 
{

  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, A5_Pin|A4_Pin|MEN2_Pin|MEN_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOA, A0_Pin|A1_Pin|A2_Pin|D13_Pin 
                          |D12_Pin|D11_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, A3_Pin|D6_Pin|D3_Pin|D5_Pin 
                          |D4_Pin|D10_Pin|D15_Pin|D14_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = A5_Pin|A4_Pin|MEN2_Pin|MEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = A0_Pin|A1_Pin|A2_Pin|D13_Pin 
                          |D12_Pin|D11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = A3_Pin|D6_Pin|D3_Pin|D5_Pin 
                          |D4_Pin|D10_Pin|D15_Pin|D14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}


void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{ 

}
#endif
