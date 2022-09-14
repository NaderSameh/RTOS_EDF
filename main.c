/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/

/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lpc21xx.h"
#include "semphr.h"
#include "event_groups.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL ((unsigned char)0x01)

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE ((unsigned long)115200)

#define PERIOD_1 50	 //task 1 period
#define PERIOD_2 50	 //task 2 period
#define PERIOD_3 100 //task 3 period
#define PERIOD_4 20	 //task 4 period
#define PERIOD_5 10	 //task 5 period
#define PERIOD_6 100 //task 6 period
#define Load_1_Simulation_Load 5
#define Load_2_Simulation_Load 12

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware(void);
/*-----------------------------------------------------------*/

/*Refrance of tasks*/

TaskHandle_t task_1_Handle = NULL;
TaskHandle_t task_2_Handle = NULL;
TaskHandle_t task_3_Handle = NULL;
TaskHandle_t task_4_Handle = NULL;
TaskHandle_t task_5_Handle = NULL;
TaskHandle_t task_6_Handle = NULL;
/*-----------------------------------------------------------*/

/*prototype for tasks*/
void task_1(void *pvParameters);
void task_2(void *pvParameters);
void task_3(void *pvParameters);
void task_4(void *pvParameters);
void task_5(void *pvParameters);
void task_6(void *pvParameters);

/*-----------------------------------------------------------*/

/*prototype for RTOS*/
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
/*-----------------------------------------------------------*/

/*globale Variable */
volatile pinState_t button_status = PIN_IS_LOW;
char timeBuffer[190];

int total_time;
int cpu_load;
int total_execution_time = 0;
int task_in;
int task_out;

unsigned char button_1 = 0;
unsigned char button_2 = 0;

QueueHandle_t MyQueue;
/*-----------------------------------------------------------*/

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
/*-----------------------------------------------------------*/

int main(void)

{

	prvSetupHardware();

	MyQueue = xQueueCreate(50, sizeof(char *));

#if (configUSE_EDF_SCHEDULER == 1)

	xTaskPeriodicCreate(task_1, (const char *)"Button_1_Monitor",
						configMINIMAL_STACK_SIZE,
						NULL, 3, NULL, PERIOD_1);
	xTaskPeriodicCreate(task_2, (const char *)"Button_2_Monitor",
						configMINIMAL_STACK_SIZE,
						NULL, 3, NULL, PERIOD_2);
	xTaskPeriodicCreate(task_3, (const char *)"Periodic_Transmitter",
						configMINIMAL_STACK_SIZE,
						NULL, 3, NULL, PERIOD_3);
	xTaskPeriodicCreate(task_4, (const char *)"Uart_Receiver",
						configMINIMAL_STACK_SIZE,
						NULL, 3, NULL, PERIOD_4);
	xTaskPeriodicCreate(task_5, (const char *)"Load_1_Simulation",
						configMINIMAL_STACK_SIZE,
						NULL, 3, NULL, PERIOD_5);
	xTaskPeriodicCreate(task_6, (const char *)"Load_2_Simulation",
						configMINIMAL_STACK_SIZE,
						NULL, 3, NULL, PERIOD_6);
	vTaskStartScheduler();
	for (;;)
		;
#endif
	return 0;
}

void task_1(void *pvParameters)
{
	pinState_t buttonNew = 0, buttonOld = 0;
	char *butRise = "Button 1 Rising\n";
	char *butFall = "Button 1 Falling\n";
	TickType_t xLastWakeTime; // variable that holds the time at which the task was last unblocked
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void *)1);


	for (;;)
	{
		buttonNew = GPIO_read(PORT_0, PIN0);
		if (buttonNew == 0 && buttonOld == 1)
		{

			if (MyQueue != 0)
			{

				xQueueSend(MyQueue, (void *)&butFall, (TickType_t)10);
			}
		}
		else if (buttonNew == 1 && buttonOld == 0)
		{
			if (MyQueue != 0)
			{

				xQueueSend(MyQueue, (void *)&butRise, (TickType_t)10);
			}
		}

		buttonOld = buttonNew;
		vTaskDelayUntil(&xLastWakeTime, PERIOD_1);
	}
}

void task_2(void *pvParameters)
{
		pinState_t buttonNew = 0, buttonOld = 0;
	char *butRise = "Button 2 Rising\n";
	char *butFall = "Button 2 Falling\n";
	TickType_t xLastWakeTime; // variable that holds the time at which the task was last unblocked
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void *)2);


	for (;;)
	{
		buttonNew = GPIO_read(PORT_0, PIN1);
		if (buttonNew == 0 && buttonOld == 1)
		{
			if (MyQueue != 0)
			{
				xQueueSend(MyQueue, (void *)&butFall, (TickType_t)10);
			}
		}
		else if (buttonNew == 1 && buttonOld == 0)
		{
			if (MyQueue != 0)
			{
				xQueueSend(MyQueue, (void *)&butRise, (TickType_t)10);
			}
		}

		buttonOld = buttonNew;
		vTaskDelayUntil(&xLastWakeTime, PERIOD_2);
	}
}
void task_3(void *pvParameters)
{

	char *periodicMessage = "Periodic Message\n";
	TickType_t xLastWakeTime; // variable that holds the time at which the task was last unblocked
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void *)3);

	for (;;)
	{

		if (MyQueue != 0)
		{
			xQueueSend(MyQueue, (void *)&periodicMessage, (TickType_t)0);
		}
		vTaskDelayUntil(&xLastWakeTime, PERIOD_3);
	}
}
void task_4(void *pvParameters)
{
		char *RxBuffer = NULL;
	TickType_t xLastWakeTime; // variable that holds the time at which the task was last unblocked
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void *)4);

	for (;;)
	{
		if (xQueueReceive(MyQueue, &RxBuffer, (TickType_t)0) == pdTRUE)
		{

			vSerialPutString((const signed char *)RxBuffer, 20);
		}
	//	xSerialPutChar('\n');
		vTaskDelayUntil(&xLastWakeTime, PERIOD_4);
	}
}
void task_5(void *pvParameters)
{
	volatile int cnt = Load_1_Simulation_Load;
	TickType_t xLastWakeTime; // variable that holds the time at which the task was last unblocked
	vTaskSetApplicationTaskTag(NULL, (void *)5);
	xLastWakeTime = xTaskGetTickCount();


	for (;;)
	{
		TickType_t tick = xTaskGetTickCount(),
				   x;
		while (cnt)
		{
			if ((x = xTaskGetTickCount()) > tick)
			{
				--cnt;
				tick = x;
			}
		}
		cnt = Load_1_Simulation_Load;
		vTaskDelayUntil(&xLastWakeTime, PERIOD_5);
	}
}

void task_6(void *pvParameters)
{
		volatile int cnt = Load_2_Simulation_Load;
	TickType_t xLastWakeTime; // variable that holds the time at which the task was last unblocked
	vTaskSetApplicationTaskTag(NULL, (void *)6);
	xLastWakeTime = xTaskGetTickCount();


	for (;;)
	{
		TickType_t tick = xTaskGetTickCount(),
				   x;
		while (cnt)
		{
			if ((x = xTaskGetTickCount()) > tick)
			{
				--cnt;
				tick = x;
			}
		}
		cnt = Load_2_Simulation_Load;
		vTaskDelayUntil(&xLastWakeTime, PERIOD_6);
	}
}

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware(void)
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/