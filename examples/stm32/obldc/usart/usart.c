/*
 * This file is part of the libopenstm32 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stm32/rcc.h>
#include <stm32/gpio.h>
#include <stm32/usart.h>

void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* Enable GPIOA clock. For LED gpio's */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);

	/* Enable clocks for GPIO port B (for GPIO_USART1_TX) and USART1. */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART1EN);
}

void usart_setup(void)
{

	/* Setup GPIO6 (in GPIO port A) to 'output push-pull' for led use. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,
		      GPIO6);

	AFIO_MAPR |= AFIO_MAPR_USART1_REMAP;

	/* Setup GPIO pin GPIO_USART3_TX/GPIO10 on GPIO port B for transmit. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_RE_TX);

	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 230400);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART1);
}

void gpio_setup(void)
{
	/* Set GPIO12 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO6);
}

int main(void)
{
	int i, j = 0, c = 0;

	clock_setup();
	gpio_setup();
	usart_setup();

	/* Blink the LED (PC12) on the board with every transmitted byte. */
	while (1) {
		gpio_toggle(GPIOA, GPIO6);	/* LED on/off */
		usart_send_blocking(USART1, c + '0');	/* Send one byte on USART3. */
		c = (c == 9) ? 0 : c + 1;	/* Increment c. */
		if ((j++ % 80) == 0) {		/* Newline after line full. */
			usart_send_blocking(USART1, '\r');
			usart_send_blocking(USART1, '\n');
		}
		for (i = 0; i < 80000; i++);	/* Wait (needs -O0 CFLAGS). */
	}

	return 0;
}
