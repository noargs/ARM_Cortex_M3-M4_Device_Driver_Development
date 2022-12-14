/**
 *  Issue an interrupt when User button goes high (pressed and released)
 *
 *  (Page references were added for F407 Discovery Board)
 *  User & WAKE-UP Button is connected to PA0 (GPIO Port A and 0th pin) - Schematic Page 31
 *
 *  How GPIO pins interrupt the processor?
 *  (How the vendor delivers the GPIO interrupt to the processor?)
 *
 *  Go to Interrupts and events > External interrupt/event controller (EXTI) >
 *  External interrupt/event line mapping - Reference Manual Page 383
 *
 *  EXTI engine hanging on APB Bus - Reference Manual Page 381, Block Diagram - DataSheet Page 16
 *  EXTI derives the clock from PCLK2 (P CLOCK 2)
 *  EXTI finally connects to NVIC interrupt controller
 *  EXTI is giving its 23 lines to NVIC (23 different IRQ numbers)
 *  Look into Vector table to confirm 23 lines are there - Reference Manual Page 373
 *  (i.e. RTC Wakeup interrupt is not connected to NVIC directly but through EXTI Line - Reference Manual Page 373)
 *  (i.e. USB On-The-Go FS OTG_FS WKUP Wakeup through EXTI line interrupt - Reference Manual Page 375)
 *
 *  GPIO Pins delivers their interrupt to NVIC through EXTI - Reference Manual Page 383
 *  PA0 to PI0 are connected to EXTI0 and register to look for is SYSCFG_EXTICR0
 *  PA1 to PI1 -> EXTI1 and register SYSCFG_EXTICR1
 *  ... ...
 *  PA15 to PI15 -> EXTI15 and register SYSCFG_EXTICR15
 *
 *  (USER button connected to PA0 -> EXTI10 (SYSCFG_EXTICR1) -> NVIC)
 *
 *  [Summary]: Button interrupt
 *  1- The button is connected to a GPIO pin of the microcontroller
 *  2- The GPIO pin should be configured to Input Mode
 *  3- The link between a GPIO port and the relevant EXTI line must be
 *     established using the SYSCFG_EXTICRx register
 *  4- Configure the trigger detection (falling edge/rising edge/both) for relevant
 *     EXTI line (done via EXTI controller register)
 *  5- Implement the handler to service the interrupt
 *
 *	[Reference Manual Page 382]
 *  - EXTI doesn't poke the NVIC unless you unmask one of the EXTI register (i.e. Pending request register
 *    Interrupt mask register, Rising trigger selection register etc)
 *  - EXTI also has Edge detection circuitry which detect the Rising Edge or Falling Edge
 *
 *  [EXTI Registers - Reference Manual Page 384]
 *  - Using 'Interrupt mask register' EXTI_IMR you can mask|unmask interrupt delivery for 23 lines
 *    from 0-22 bits (cover 23 lines) are applicable and rest reserved
 *
 */

#include <stdio.h>
#include <stdint.h>

// global shared variable between main code and ISR
uint8_t volatile g_button_pressed = 0;

uint32_t g_button_press_count = 0;

void button_init(void);

uint32_t volatile *exti_pending_reg     = (uint32_t*) (0x40013C00 + 0x14); // Base address - Reference Manual Page 65
uint32_t volatile *clk_ctrl_reg         = (uint32_t*) (0x40023800 + 0x30);
uint32_t volatile *clk_ctrl_reg_apb2    = (uint32_t*) (0x40023800 + 0x44);
uint32_t volatile *gpioa_mode_reg       = (uint32_t*) (0x40020000 + 0x00);
uint32_t volatile *exti_mask_reg        = (uint32_t*) (0x40013C00 + 0x00);
uint32_t volatile *exti_edge_ctrl_reg   = (uint32_t*) (0x40013C00 + 0x08);
uint32_t volatile *nvic_irqen_reg       = (uint32_t*) 0xE000E100;

int main(void)
{
	button_init();
	while(1){
		// disable interrupt
		*exti_mask_reg &= ~(1 << 0);
		if(g_button_pressed) {
			// some delay until button debounce gets over
			for(uint32_t volatile i=0;i<500000/2;i++);
			g_button_press_count++;
			printf("Button is pressed\n"); // printf("button is pressed: %lu\n", g_button_press_count);
			g_button_pressed = 0;
		}

		// enable interrupt
		*exti_mask_reg |= (1 << 0);
	}
}

// configuration of gpio pin, exti edge detection, masking and NVIC irq enable
void button_init(void) {
	// GPIOA clock enable
	*clk_ctrl_reg |= (1 << 0);

	// SYSCFG clock enable
	*clk_ctrl_reg_apb2 |= (1 << 14);

	// Edge detection configuration
	*exti_edge_ctrl_reg |= (1 << 0);

	// EXTI interrupt enable
	*exti_mask_reg |= (1 << 0);

	// NVIC irq enable
	*nvic_irqen_reg |= (1 << 6);
}


void EXTI0_IRQHandler(void) {
	// make this flag set if button pressed
	g_button_pressed = 1;

	// clearing of EXTI interrupt pending bit
	// [Caution] if you don't clear this bit with 1, there will be infinite interrupts
	*exti_pending_reg |= (1 << 0);
}




























