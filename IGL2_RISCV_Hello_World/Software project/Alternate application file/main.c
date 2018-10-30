/*******************************************************************************
 * (c) Copyright 2016-2017 Microsemi SoC Products Group. All rights reserved.
 *
 * This SoftConsole example project demonstrates how to use configure and use the
 * CoreRISCV_AXI4 system timer.
 *
 * Please refer README.TXT in the root folder of this project for more details.
 */
#include "riscv_hal.h"
#include "hw_platform.h"
#include "core_uart_apb.h"
#include "core_gpio.h"
#include "core_timer.h"

const char *g_hello_msg = "\r\nHello World!\n\r";

/*-----------------------------------------------------------------------------
 * UART instance data.
 */
UART_instance_t g_uart;

/*-----------------------------------------------------------------------------
 * GPIO instance data.
 */
gpio_instance_t g_gpio_in;
gpio_instance_t g_gpio_out;

/*-----------------------------------------------------------------------------
 * Timer instance data.
 */
timer_instance_t g_timer0;
timer_instance_t g_timer1;

/*-----------------------------------------------------------------------------
 * Global state counter.
 */
uint32_t g_state;

/*-----------------------------------------------------------------------------
 * System Tick interrupt handler
 */
void SysTick_Handler(void) {

    uint32_t gpout;

    gpout = GPIO_get_inputs(&g_gpio_in);

    g_state = g_state << 1;
    if (g_state > 8) {
        g_state = 0x01;
    }
    gpout = gpout | g_state;

    GPIO_set_outputs(&g_gpio_out, gpout);
}

/*-----------------------------------------------------------------------------
 * Core Timer 1 Interrupt Handler
 */
void External_31_IRQHandler() {

  uint32_t gpout;

  gpout = GPIO_get_inputs(&g_gpio_in);

  g_state = g_state << 1;
  if (g_state > 8) {
      g_state = 0x01;
  }
  gpout = gpout | g_state;

  GPIO_set_outputs(&g_gpio_out, gpout);

  TMR_clear_int(&g_timer1);
  PLIC_CompleteIRQ(PLIC_ClaimIRQ());
}

/*-----------------------------------------------------------------------------
 * Core Timer 0 Interrupt Handler
 */
void External_30_IRQHandler() {

  uint32_t gpout;

  gpout = GPIO_get_inputs(&g_gpio_in);

  g_state = g_state >> 1;
  if (g_state == 0) {
      g_state = 0x0A;
  }
  gpout = gpout | g_state;

  GPIO_set_outputs(&g_gpio_out, gpout);
  
  TMR_clear_int(&g_timer0);
  PLIC_CompleteIRQ(PLIC_ClaimIRQ());
}

/*-----------------------------------------------------------------------------
 * main
 */
int main(int argc, char **argv) {

	uint8_t rx_char;
  uint8_t rx_count;

  uint32_t pb_input;

  g_state = 0x0A;

  PLIC_init();

  PLIC_SetPriority(TIMER0_IRQn, 1);
  PLIC_EnableIRQ(TIMER0_IRQn);

  PLIC_SetPriority(TIMER1_IRQn, 1);
  PLIC_EnableIRQ(TIMER1_IRQn);
		
  GPIO_init(&g_gpio_in, COREGPIO_IN_BASE_ADDR, GPIO_APB_32_BITS_BUS);
  GPIO_init(&g_gpio_out, COREGPIO_OUT_BASE_ADDR, GPIO_APB_32_BITS_BUS);

  UART_init(&g_uart,
            COREUARTAPB0_BASE_ADDR,
						BAUD_VALUE_115200,
            (DATA_8_BITS | NO_PARITY));

  UART_polled_tx_string(&g_uart, (const uint8_t *)g_hello_msg);

	// Initiate Timers
  TMR_init(&g_timer0,
           CORETIMER0_BASE_ADDR,
					 TMR_CONTINUOUS_MODE,
					 PRESCALER_DIV_1024, // (66MHZ / 1024) ~ 64.5kHz
					 21500); // (64.5kHz / 21500) ~ 0.33sec

  TMR_init(&g_timer1,
           CORETIMER1_BASE_ADDR,
					 TMR_CONTINUOUS_MODE,
					 PRESCALER_DIV_1024, // (66MHZ / 1024) ~ 64.5kHz
					 32250); // (64.5kHz / 32250) ~ 0.5sec

	// Enable the timers IRQs...
  TMR_enable_int(&g_timer0);
  TMR_enable_int(&g_timer1);

  // Start the timer
  TMR_start(&g_timer0);

  // Enable external IRQs
  __enable_irq();


  //SysTick_Config(SYS_CLK_FREQ / 2);

  /*
   * Loop. Echo back characters received on UART.
   */
  do {
    rx_count = UART_get_rx(&g_uart, &rx_char, 1);
    if (rx_count > 0) {
      UART_send(&g_uart, &rx_char, 1);
    }

    pb_input = GPIO_get_inputs(&g_gpio_in);

    if (pb_input == 0x01) {
    	g_state = 0x01;
    	TMR_stop(&g_timer0);
    	TMR_start(&g_timer1);
    }
    if (pb_input == 0x02) {
    	g_state = 0x0A;
    	TMR_stop(&g_timer1);
    	TMR_start(&g_timer0);
    }

  } while (1);

  return 0;
}

