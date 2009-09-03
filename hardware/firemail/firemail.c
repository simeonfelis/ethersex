/*
*
* Copyright (c) 2009 by Simeon Felis <simeonfelis@googlemail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#include <stdio.h>
#include <stdlib.h>
#include "smtp.h"
#include "firemail.h"
#include "protocols/uip/uip.h"
//#include "core/usart.h"


int uart_putchar(char c, FILE *stream);
static FILE fmstdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static btn_state  = FM_BTN_NOT_PRESSED;

#define STATE (&uip_conn->appstate.firemail)

int 
uart_putchar( char c, FILE *stream )
{
    if( c == '\n' )
        uart_putchar( '\r', stream );
 
    loop_until_bit_is_set( UCSRA, UDRE );
    UDR = c;
    return 0;
}

void
fm_btn_poll (void)
{
    if (FM_IS_BTN_PUSHED) {
        if (btn_state == FM_BTN_GOT_PRESSED)
            btn_state = FM_BTN_IS_PRESSED;
        if (btn_state == FM_BTN_NOT_PRESSED) 
            btn_state = FM_BTN_GOT_PRESSED;
    }
    else {
        if (btn_state == FM_BTN_RELEASED)
            btn_state = FM_BTN_NOT_PRESSED;
        if (btn_state == FM_BTN_IS_PRESSED)
            btn_state = FM_BTN_RELEASED;
    }
}

//generate_usart_init(); // Helping Makro


void 
uart_init(void)
{
// Hilfsmakro zur UBRR-Berechnung ("Formel" laut Datenblatt)
#define UART_UBRR_CALC(BAUD_,FREQ_) ((FREQ_)/((BAUD_)*16L)-1)
 
    UCSRB |= (1<<TXEN) | (1<<RXEN);    // UART TX und RX einschalten
    UCSRC |= (1<<URSEL)|(3<<UCSZ0);    // Asynchron 8N1 
 
    UBRRH = (uint8_t)( UART_UBRR_CALC( UART_BAUD_RATE, F_CPU ) >> 8 );
    UBRRL = (uint8_t)UART_UBRR_CALC( UART_BAUD_RATE, F_CPU );
}
void
firemail_send_data (uint8_t send_state)
{
}


void
firemail_main(void)
{
}

void 
firemail_periodic(void) 
{
    fm_btn_poll();
}

void 
firemail_init(void) 
{
    FM_LED_OFF;
    FM_LED_ACTIVATE;
    FM_BTN_ACTIVATE;
    uart_init();
    stdout = &fmstdout;

	uint8_t server_ip[] = {192, 168, 178, 27};
	
	smtp_init();
	smtp_configure("firemail", (void *)server_ip);
}


/*
  -- Ethersex META --
  header(hardware/firemail/firemail.h)
  timer(500, firemail_periodic)
  init(firemail_init)
*/

