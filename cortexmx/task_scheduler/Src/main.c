/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

#include<stdio.h>
#include "main.h"
#include "led.h"

/* These tasks will be scheduled by our scheduler in round robin fashion */
void task1_handler(void);	// This is task1
void task2_handler(void);	// This is task2
void task3_handler(void);	// This is task3
void task4_handler(void);	// This is task4

void init_systick_timer(uint32_t tick_hz);	// systick conf. function declaration
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack);
void init_tasks_stack(void);	// prototype
void enable_processor_faults(void);
__attribute__((naked)) void switch_sp_to_psp(void);
uint32_t get_psp_value(void);

// should mention for how many ticks it wished to get blocked
void task_delay(uint32_t tick_count);


// global variable to store a PSP values of different task
// is an array of MAX_TASKS

// uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START, T2_STACK_START, T3_STACK_START, T4_STACK_START};

// global variable to store different task handler's address

// uint32_t task_handlers[MAX_TASKS];

/**
 * make current_task = 1 becaule 0 should be idle task
 * you should always start with task 1
 * idle task sould run when all task are blocked
 */
uint8_t current_task = 1;	// value = 0 means task1 is running
uint32_t g_tick_count = 0;  // global tick count

typedef struct {
	uint32_t psp_value;
	uint32_t block_count;
	uint8_t  current_state;
	void (*task_handler)(void);	// funtion pointer to hold the address to the task
}TCB_t;

TCB_t user_tasks[MAX_TASKS];

int main(void)
{

	// we are dealing with stack memory and we may
	// be touching or we may doing some illegal activities related to memory
	// or inline assembly or whatever it could be, or there may be fault while changing from
	// handler to thread mode or something like that.
	// In order to trace the fault!
	enable_processor_faults();

	/* here use MSP as stack pointer by default */

	init_scheduler_stack(SCHED_STACK_START);	// argument no.1 stored in R0

	/*
	task_handlers[0] = (uint32_t)task1_handler;
	task_handlers[1] = (uint32_t)task2_handler;
	task_handlers[2] = (uint32_t)task3_handler;
	task_handlers[3] = (uint32_t)task4_handler;
	*/

	init_tasks_stack();

	led_init_all();	// initializa all LEDs

	init_systick_timer(TICK_HZ);

	/* before call task1_handler, need to transfer to PSP as stack pointer */
	switch_sp_to_psp();

	task1_handler();

	for(;;);
}

/**
 * idle task is nothing but an another task
 */
void idle_task(void)
{
	while(1);
}


void task1_handler(void)
{
	while(1)
	{
//		printf("This is task1\n");
		led_on(LED_BLUE);
		task_delay(1000);	// this task will get blocked for the next 1000 ticks
//		delay(DELAY_COUNT_1S);
		led_off(LED_BLUE);
		task_delay(1000);
//		delay(DELAY_COUNT_1S);
	}
}
void task2_handler(void)
{
	while(1)
	{
//		printf("This is task2\n");
		led_on(LED_RED);
		task_delay(500);	// this task will get blocked for the next 500 ticks
//		delay(DELAY_COUNT_500MS);
		led_off(LED_RED);
//		delay(DELAY_COUNT_500MS);
		task_delay(500);
	}
}
void task3_handler(void)
{
while(1)
	{
		printf("This is task3\n");
	}
}
void task4_handler(void)
{
while(1)
	{
		printf("This is task4\n");
	}
}

void init_systick_timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014;	// SysTick Reload Value Register in generic user guide ch4.4
	uint32_t *pSCSR = (uint32_t*)0xE000E010;

	uint32_t count_value = (SYSTICK_TIM_CLK / tick_hz) - 1;	// remember to minus 1

	// Clear the value of SVR
	*pSRVR &= ~(0x00FFFFFF);	// only 24 bits are valid

	// load the value in to SVR
	*pSRVR |= count_value;

	// do some settings
	*pSCSR |= (1 << 1); 	// Enables SysTick exception request
	*pSCSR |= (1 << 2); 	// Indicates the clock source, processor clock source

	// enable the setting
	*pSCSR |= (1 << 0);		// Enables the counter
}

__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack)
{
	/* change the value of the MSP */
	__asm volatile("MSR MSP,%0": :  "r" (sched_top_of_stack)  : );
	__asm volatile("BX LR");	// go back to main function
}




void init_tasks_stack(void)
{
	// keep all the task in a running state
	user_tasks[0].current_state = TASK_READY_STATE;
	user_tasks[1].current_state = TASK_READY_STATE;
	user_tasks[2].current_state = TASK_READY_STATE;
	user_tasks[3].current_state = TASK_READY_STATE;
	user_tasks[4].current_state = TASK_READY_STATE;

	// initialize psp value
	user_tasks[0].psp_value = IDLE_STACK_START;
	user_tasks[1].psp_value = T1_STACK_START;
	user_tasks[2].psp_value = T2_STACK_START;
	user_tasks[3].psp_value = T3_STACK_START;
	user_tasks[4].psp_value = T4_STACK_START;

	// initialize function pointer
	user_tasks[0].task_handler = idle_task;
	user_tasks[1].task_handler = task1_handler;
	user_tasks[2].task_handler = task2_handler;
	user_tasks[3].task_handler = task3_handler;
	user_tasks[4].task_handler = task4_handler;


	uint32_t *pPSP;

	for(int i = 0; i < MAX_TASKS; i++)
	{
		pPSP = (uint32_t*) user_tasks[i].psp_value;

		/* fully descending */
		pPSP--;	// xPSR
		*pPSP = DUMMY_XPSR;	// 0x00100000

		pPSP--;	// PC
		*pPSP =  (uint32_t) user_tasks[i].task_handler;

		pPSP--;	// LR
		*pPSP = 0xFFFFFFFD;

		for(int j = 0; j < 13; j++){
			pPSP--;
			*pPSP = 0; // from R12 to R4
		}

		user_tasks[i].psp_value = (uint32_t)pPSP; // preserve the PSP value, very important
	}
}

void enable_processor_faults(void)
{
	uint32_t *pSHCRS = (uint32_t*)0xE000ED24;

	*pSHCRS |= ( 1 << 16); // mem manage
	*pSHCRS |= ( 1 << 17); // bus fault
	*pSHCRS |= ( 1 << 18); // usage fault
}


void save_psp_value(uint32_t current_psp_value)
{
	user_tasks[current_task].psp_value = current_psp_value;
	//	psp_of_tasks[current_task] = current_psp_value;
}

void update_next_task(void)
{
	int state = TASK_BLOCKED_STATE;
	// if all user_tasks are in blocked state,
	// then this state variable will not get modified.

	for(int i = 0; i < (MAX_TASKS); i++) {	// go through all the task
		current_task++;
		current_task %= MAX_TASKS;	// 0 -> 1 -> 2 -> 3 -> 4 -> 0 ...
		state = user_tasks[current_task].current_state;
		if((state == TASK_READY_STATE) && (current_task != 0))	// if it is ready, then it is schedulable (ignore idle task)
			break;
	}
	if(state != TASK_READY_STATE)	// all the tasks are in blocked state.
		current_task = 0;	// idle_task
}



uint32_t get_psp_value(void)
{
	return user_tasks[current_task].psp_value;
	// return psp_of_tasks[current_task];
}


__attribute__((naked)) void switch_sp_to_psp(void)	// change SP to PSP need inline assembly "MRS"
{
	// 1. initializer the PSP with TASK1 stack start

	// get the value of PSP of current_task
	__asm volatile ("PUSH {LR}");	// preserve LR which connects back to main
	__asm volatile ("BL get_psp_value");	// the code will branch to get_psp_value
	__asm volatile ("MSR PSP,R0");	// initialize PSP
	__asm volatile ("POP {LR}");	// pops back LR value

	// 2. change SP to PSP using CONTROL register
	__asm volatile ("MOV R0,#0x02");	// use MOV instruction to store some immediate value
	__asm volatile ("MSR CONTROL,R0");	// push R0 to CONTROL register
	__asm volatile ("BX LR");
}


void schedule()
{
	// pend the pendSV exception
	uint32_t *pICSR = (uint32_t*)0xE000ED04;
	*pICSR |= (1 << 28);
}




void task_delay(uint32_t tick_count)
{

	// disable interrupt
	INTERRUPT_DISABLE();

	// take global tick count (g_tick_count) as a reference (maintain by the sysTick)
	// tick_count is sent by the task
	if(current_task) {	// only blocked user task (0 means idle task)
		user_tasks[current_task].block_count = g_tick_count + tick_count;	// update the block count
		user_tasks[current_task].current_state = TASK_BLOCKED_STATE;	// change the state
		schedule();	// allow other task to run
	}

	// enable interrupt
	INTERRUPT_ENABLE();
}

__attribute__((naked)) void PendSV_Handler(void)
{
	/* Save the context of current task */
	// 1. Get current running task's PSP value
	__asm volatile ("MRS R0, PSP");
	// 2. Using that PSP value store SF2 (R4 to R11)
	__asm volatile ("STMDB R0!,{R4-R11}");

	__asm volatile ("PUSH {LR}");	// save LR value

	// 3. Save the current of PSP
	__asm volatile ("BL save_psp_value");	//LR will going to change here


	/* Retrieve the context of next task */
	// 1. Decide next task to run

	__asm volatile ("BL update_next_task");	//LR will going to change here

	// 2. get its past PSP value
	__asm volatile ("BL get_psp_value");	//LR will going to change here

	// 3. Using that PSP value retrieve SF2 (R4 to R11)
	__asm volatile ("LDM R0!, {R4-R11}");

	// 4. update PSP and exit
	__asm volatile ("MSR PSP, R0");

	__asm volatile ("POP {LR}");	// retrieve LR value

	__asm volatile("BX LR");	// write exception exit manually.
}

void update_global_tick_count(void)
{
	g_tick_count++;
}

/**
 * check blocked task which can qualify for a running state.
 * (by comparing the block_tick_count or block period of the bloccked_task)
 */
void unblock_tasks(void)
{
	for(int i = 1; i < MAX_TASKS; i++) {	// i = 0 is idle_task, so don't need to check
		if(user_tasks[i].current_state != TASK_READY_STATE) {	// not ready state
			if(user_tasks[i].block_count == g_tick_count) {	// means that the delay is elapsed
				user_tasks[i].current_state = TASK_READY_STATE;
			}
		}
	}
}


void SysTick_Handler(void)	// not to be naked function, treat it as a C funtion
{
	uint32_t *pICSR = (uint32_t*)0xE000ED04;

	update_global_tick_count();
	unblock_tasks();	// check blocked task which can qualify for a running state. (by comparing the block_tick_count or block period of the bloccked_task)
	// pend the pendSV exception
	*pICSR |= (1 << 28);
}

void HardFault_Handler(void)
{
	printf("Exception : Hardfault\n");
	while(1);	// hang
}
void MemManage_Handler(void)
{
	printf("Exception : MemManage\n");
	while(1);	// hang
}
void BusFault_Handler(void)
{
	printf("Exception : BusFault\n");
	while(1);	// hang
}


