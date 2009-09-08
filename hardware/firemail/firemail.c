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
#include <string.h>
#include "firemail.h"
#include "firemail_state.h"
#include "protocols/uip/uip.h"
//#include "core/usart.h"


int uart_putchar(char c, FILE *stream);
static FILE fmstdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static btn_state;
uip_conn_t *conn;
uip_ipaddr_t server_ip;

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
firemail_connect()
{
    printf("\nTrying to connect...\n");
    conn = uip_connect(&server_ip, HTONS(25), firemail_main);
    if (conn == NULL) {
        printf ("uip_connect failed\n");
        return;
    }
}

void
firemail_main(void)
{
    static char *buffer;
    static uint8_t buflen;

    if (uip_aborted() || uip_timedout()) {
        printf("aborted\n");
        conn = NULL;
    }
    
    if (uip_closed()) {
        printf("closed\n");
        conn = NULL;
    }

    if (uip_rexmit()) {
        printf("data needs to be resent\n");
    }
    else if (uip_connected() || uip_newdata() || uip_acked() ) {
        printf("acked, newdata, connected\n");
        firemail_send_data();
    }
    else if (uip_poll()) {
        printf("polled\n");
    }
    
    

    buffer = uip_appdata;
    buflen = 13;
    sprintf_P (buffer, "HELO a@abc.de", buflen);
    uip_send(uip_appdata, buflen);
}

void 
firemail_periodic(void) 
{
    fm_led_poll();
    fm_btn_poll();
}

void 
firemail_init(void) 
{
    uart_init();
    fm_btn_init();
    fm_led_init();

    stdout = &fmstdout;

    uip_ipaddr (server_ip, 192,168,178,27);
    firemail_connect();
}

void
fm_led_init()
{
    FM_LED_OFF;
    FM_LED_ACTIVATE;
}

void
fm_led_poll()
{
}

void
fm_btn_init()
{
   FM_BTN_ACTIVATE;
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



/*
  -- Ethersex META --
  header(hardware/firemail/firemail.h)
  timer(500, firemail_periodic)
  init(firemail_init)
*/

