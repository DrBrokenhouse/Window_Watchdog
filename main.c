/****************************************************************
 * Robert Thomas
 * 3/3/2014
 * Window Watchdog Tutorial
 */
//**********************Libraries*********************************
#include <stm32F4xx.h>
#include <stm32f4xx_wwdg.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32F4xx_it.h>

//********************** Prototype functions ************************
void WWDG_Init(void); // Window Watchdog setup
void DelayMs(__IO uint32_t time); //Delay using systick
void FeedDog(float round); // Window Watchdog delay and dog food.


//****************** global variables for program *****************************
extern __IO uint32_t TimeDelay; // This varibale is found in stm32f4xx_it.c

//********************** Systick Delay setup ************************
void DelayMs(__IO uint32_t time)
{
	TimeDelay = time;
  while(TimeDelay != 0);
}

//********************** STM32F peripheral setup ************************

void setup_Periph(void)
{
	// Port initialization
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable the GPIOD clock for LEDs
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Setup for GPIOD pins for LEDS

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

//*********************** WWDG configuration ********************************
void WWDG_Init(void)
{
	// Enable WWDG clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

	// WWDG clock counter = (PCLK1 (42MHz)/4096)/8 = 1281 Hz (~780 us)
	WWDG_SetPrescaler(WWDG_Prescaler_8);

	// Set Window value to 80; WWDG counter should be refreshed only when the counter
	// is below 127 (and greater than 64) otherwise a reset will be generated
	WWDG_SetWindowValue(80);

	//  Enable WWDG and set counter value to 127, WWDG timeout = ~780 us * 64 = 49.92ms
	// this case the refresh window is:
	// ~780 * (127-80) = 37ms < refresh window < ~780 * 64 = 49.9ms

	WWDG_Enable(127);
}

//********************* main() ********************************
int main(void)
{
	// Setup the board
	setup_Periph();

	// make sure the clock is stable
	RCC_HSEConfig(RCC_HSE_ON);
	while(!RCC_WaitForHSEStartUp());

	// Set up the WWDG
	WWDG_Init();
	// Systick Configuration
	SysTick_Config(SystemCoreClock/1000); // 1 mill. Sec.


	while(1)// Principle program loop
    {
		// Check if the system has resumed from WWDG reset
	  if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET) // if 1
	  {
		  // WWDGRST flag set and PD0 is used to indicates that there is a reset.
		  // Green LED and Red LED are on.
		 // GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		 // NVIC_SystemReset();
		  // Clear reset flags
		  RCC_ClearFlag();

	  }
	  else // if 0
	  {
		  // WWDGRST flag is not set and PD0 in is off to indicate that there isn't a reset
		  GPIO_ResetBits(GPIOD, GPIO_Pin_0);
	  }
		  //WWDG_SetCounter(127); // This refreshes the watchdog between 0ms and 36.6ms

		// Test point, Amber LED, and Blue LED are on.
		GPIO_SetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_15);
		// Green LED and Red LED are off.
		GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_14);
		//DelayMs(2);// This causes and additional 2ms which pushes the FeedDog() past 49.9ms

		// See comment in FeedDog()
		FeedDog(3); // FeedDog(n) n = n * 49.9ms: So FeedDog(3) = 149.7ms

		// Green LED and Red LED are on.
		GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_14);
		// Test point, Amber LED, and Blue LED are off.
		GPIO_ResetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_15);
	// DelayMs(2);// This causes and additional 2ms which pushes the FeedDog() past 49.9ms

		// See comment in FeedDog()
		FeedDog(3); // FeedDog(n) n = n * 49.9ms: So FeedDog(3) = 149.7ms

   }
}

void FeedDog(float round)
{
	// Each round adds 49.9ms of delay and refreshes the WWDG control register.
	// The delay MUST be between 37ms and 49.9ms. Anything outside of this will RESET.
	/* The formula indicates that 36.6ms is the lower limit for the refresh window.
	   The formula does not take into account for the small delay of 0.3ms used by the program
	   this point. */
	while(round)
	{
	DelayMs(49.9);// The maximum value before a reset
	WWDG_SetCounter(127);
	round--;
	}
}





























