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
#include "firemail.h"
#include "protocols/uip/uip.h"
//#include "core/usart.h"

static const char PROGMEM fm_example[] = 
    "\nHi there at rev 2\n"
    "Establishing connection...\n";

static const char PROGMEM fm_txt_helo[] = 
    "HELO presi.whitehouse.gov";

int uart_putchar(char c, FILE *stream);
static FILE fmstdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static btn_state  = FM_BTN_NOT_PRESSED;

#define FM_SEND(str) do { 						\
	memcpy_P (uip_sappdata, str, sizeof (str)); \
	uip_send (uip_sappdata, sizeof (str) -1); 	\
	} while(0);

#define STATE (&uip_conn->appstate.firemail)

static uip_conn_t *fm_conn = NULL;

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
    printf("send_data: %d\n", send_state);
    switch (send_state) {
    case FM_OPEN_STREAM:
	    FM_SEND (fm_txt_helo);
    break;
	case FM_CONNECTED:
		if (*STATE->outbuf) {
			uip_slen = sprintf_P (uip_sappdata, 
				PSTR ("HELO abc@abc.com"),
				STATE->target, STATE->outbuf);
		} else {
			printf("idle\n");
		}
	break;
    }
}


void
firemail_main(void)
{
    if (uip_aborted() || uip_timedout()) {
        printf("connection aborted\n");
        fm_conn = NULL;
    }
    if (uip_closed()) {
        printf("connection closed\n");
        fm_conn = NULL;
    }
    if (uip_connected()) {
        printf("new connection\n");
        STATE->stage = FM_OPEN_STREAM;
        STATE->sent = FM_INIT;
        strcpy_P (STATE->target, PSTR("presi.whitehouse.gov"));
        strcpy_P (STATE->outbuf, fm_txt_helo );
    }
    if (uip_acked() && STATE->stage == FM_CONNECTED) {
        printf("acked && connected\n");
        *STATE->outbuf = 0;
    }

    if (uip_newdata() && uip_len) {
        ((char *) uip_appdata)[uip_len] = 0;
        printf("received: %s\n", (char *)uip_appdata);
    }
    if (uip_rexmit()) {
		printf("reximitting\n");
        firemail_send_data (STATE->sent);
    }
    else if ((STATE->stage > STATE->sent || STATE->stage == FM_CONNECTED)
            && (uip_newdata() || uip_acked() || uip_connected())) {
        printf("elseif 1\n");
        firemail_send_data (STATE->stage);
    }
    else if (STATE->stage == FM_CONNECTED && uip_poll() && *STATE->outbuf) {
        printf("elseif 2\n");
        firemail_send_data(STATE->stage);
    }
	if (uip_acked())
		printf ("acked\n");
}

void 
firemail_periodic(void) 
{
    fm_btn_poll();
    if (!fm_conn) {
		printf("connection lost\n");
        firemail_init();
	}
}

void 
firemail_init(void) 
{
    FM_LED_OFF;
    FM_LED_ACTIVATE;
    FM_BTN_ACTIVATE;
    uart_init();
    stdout = &fmstdout;
    printf("%s", fm_example);
    /* make a connection */
    uip_ipaddr_t fm_serv;
    uip_ipaddr(&fm_serv, 192,168,178,27);
    fm_conn = uip_connect(&fm_serv, HTONS(25), firemail_main);

	if (!fm_conn) {
		printf("no uip_conn available\n");
		return;
	}
}


/*
  -- Ethersex META --
  header(hardware/firemail/firemail.h)
  timer(500, firemail_periodic)
  init(firemail_init)
*/

