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
#include <stdio.h>


int fun_divide(int x, int y)
{
	return x / y;
}

int main(void)
{
	// 1. enable all configurable exceptions like usage fault, mem manage fault and bus fault
	uint32_t *pSHCRS = (uint32_t*)0xE000ED24;

	*pSHCRS |= ( 1 << 16); // mem manage
	*pSHCRS |= ( 1 << 17); // bus fault
//	*pSHCRS |= ( 1 << 18); // usage fault

	// 2. enable divide by zero trap
	uint32_t *pCCR = (uint32_t*)0xE000ED14;	// CCR address
	*pCCR |= ( 1 << 4);

	// 3. attempt ot divide by zero
	fun_divide(10, 0);

	for(;;);
}

// 2. implement the fault handlers

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

__attribute__ ((naked)) void UsageFault_Handler(void)
{
	__asm ("MRS r0,MSP");
	__asm ("B UsageFault_Handler_c");	// jumps to function UsageFault_Handler_c
}


void UsageFault_Handler_c(uint32_t *pBaseStackFrame)
{
	uint32_t *pUFSR = (uint32_t*)0xE000ED2A;
	printf("Exception : UsageFault\n");
	printf("UFSR = %lx\n", (*pUFSR) & 0xFFFF);	// use only the first 16 bits
	printf("pBaseStackFrame = %p\n", pBaseStackFrame);
	printf("Value of R0 = %lx\n", pBaseStackFrame[0]);
	printf("Value of R1 = %lx\n", pBaseStackFrame[1]);
	printf("Value of R2 = %lx\n", pBaseStackFrame[2]);
	printf("Value of R3 = %lx\n", pBaseStackFrame[3]);
	printf("Value of R12 = %lx\n", pBaseStackFrame[4]);
	printf("Value of LR = %lx\n", pBaseStackFrame[5]);
	printf("Value of PC = %lx\n", pBaseStackFrame[6]);
	printf("Value of XPSR = %lx\n", pBaseStackFrame[7]);

	while(1);	// hang
}

