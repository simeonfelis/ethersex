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
static uint16_t debug_counter;

#define CONF_FM_FROM_ADDRESS "feuer@firemail.de"
#define CONF_FM_MAIL_SERVER "yamato.local"
#define CONF_FM_REGISTRATION_NAME "slave"
#define CONF_FM_TO_ADDRESS "administrator@yamato.local"

static const char PROGMEM TXT_HELO[] = 
    "HELO "CONF_FM_REGISTRATION_NAME"."CONF_FM_MAIL_SERVER"\n";
static const char PROGMEM TXT_MAIL[] =
    "MAIL FROM:"CONF_FM_FROM_ADDRESS"\n";
static const char PROGMEM TXT_RCPT[] = 
    "RCPT TO:"CONF_FM_TO_ADDRESS"\n";
static const char PROGMEM TXT_DATA[] = 
    "DATA\n";
static const char PROGMEM TXT_TRANSMIT[] = 
    "From: <"CONF_FM_FROM_ADDRESS">\n" 
    "To: <"CONF_FM_TO_ADDRESS">\n" 
    "Subject: Feuermeldung\n" 
    "\n" 
    "Firemail meldet feuer!\n" 
    ".\n";

static uint8_t please_send_mail = 0;

#define STATE (&uip_conn->appstate.firemail)
//struct firemail_connection_state_t *fms;

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
firemail_send_mail()
{
    if (STATE->processing)
        return;

    STATE->processing = 1;

    STATE->stage = FM_INIT;
    STATE->sent = FM_INIT;
    firemail_connect();
}

static uint8_t
firemail_receive (void)
{
    /* FIXME: more secure check necessary! */
    printf("rec: %s\n", (char *)uip_appdata);
    switch (STATE->stage) {
    case FM_INIT:
        if (strstr_P (uip_appdata, PSTR("220")) == NULL) {
            printf("rec: init failed: %s\n", (char *)uip_appdata);
            return 1;
        }
        break;
    case FM_HELO:
    case FM_MAIL:
    case FM_RCPT:
    case FM_TRANSMIT:
        if (strstr_P (uip_appdata, PSTR("250")) == NULL) {
           printf("rec: failed: %s\n", (char *)uip_appdata);
           return 1;  
        }
        break;
    case FM_DATA:
        if (strstr_P (uip_appdata, PSTR("354")) == NULL) {
            printf("rec: DATA failed: %s\n", (char *)uip_appdata);
            return 1;
        }
        break;
    default:
        printf("rec: unsupported stage: %i\n", STATE->stage);
        return 1;
        break;
    }
    STATE->stage++;
    printf("rec: stage now %i; sent is %i\n", STATE->stage, STATE->sent);
    return 0;
}

void
firemail_send_data (uint8_t send_state)
{
    printf("send: stage is: %i\n", send_state);

    switch (send_state) {
    case FM_INIT:
        /* just connected to server. nothing to send */
        break;
    case FM_HELO:
        memcpy_P (uip_sappdata, TXT_HELO, sizeof(TXT_HELO));
        uip_send(uip_sappdata, sizeof(TXT_HELO) - 1);
        break;
    case FM_MAIL:
        memcpy_P (uip_sappdata, TXT_MAIL, sizeof(TXT_MAIL));
        uip_send(uip_sappdata, sizeof(TXT_MAIL) - 1);
        break;
    case FM_RCPT:
        memcpy_P (uip_sappdata, TXT_RCPT, sizeof(TXT_RCPT));
        uip_send(uip_sappdata, sizeof(TXT_RCPT) -1);
        break;
    case FM_DATA:
        memcpy_P (uip_sappdata, TXT_DATA, sizeof(TXT_DATA));
        uip_send(uip_sappdata, sizeof(TXT_DATA) -1);
        break;
    case FM_TRANSMIT:
        memcpy_P (uip_sappdata, TXT_TRANSMIT, sizeof(TXT_TRANSMIT));
        uip_send(uip_sappdata, sizeof(TXT_TRANSMIT) -1);
        break;
    case FM_FINISHED:
        break;
    default:
        printf("send_data unsupported command: %i\n", send_state);
        return;
        break;
    }
    STATE->sent = send_state;
    printf("send: sent now %i\n", send_state);
    printf("senddata: %s\n", uip_sappdata);
}

void
firemail_connect()
{
    printf("\nTrying to connect...\n");
    uip_ipaddr (server_ip, 192,168,178,27);
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

    if (uip_acked()) {
        printf("acked\n");
        *STATE->outbuf = 0;
    }

    if (uip_newdata() && uip_len) {
        ((char *) uip_appdata)[uip_len] = 0;
        if (firemail_receive()) {
            printf("received weired stuff. closing\n");
            uip_close();
            return;
        }
    }

    if (uip_rexmit()) {
        printf("resent\n");
        firemail_send_data(STATE->sent);
    }
    else if ((STATE->stage > STATE->sent) && 
             (uip_connected() || uip_acked() || uip_newdata())) {
        printf("connected....\n");
        firemail_send_data(STATE->stage);
    }
    else if (uip_poll()) {
        /* we may do anything now. it's our turn. */
        if (STATE->stage == FM_FINISHED && STATE->sent == FM_FINISHED ) {
             STATE->processing = 0;
             printf("Sendig mail done\n");
             uip_close();
        }
    }
    
    
/*
    buffer = uip_appdata;
    buflen = 13;
    sprintf_P (buffer, "HELO a@abc.de", buflen);
    uip_send(uip_appdata, buflen);
*/
}

void 
firemail_periodic(void) 
{
    fm_led_poll();
    fm_btn_poll();
    if (btn_state == FM_BTN_GOT_PRESSED )
        firemail_send_mail();
    else if (btn_state == FM_BTN_IS_PRESSED)
        FM_LED_ON;
    else if (btn_state == FM_BTN_NOT_PRESSED)
        FM_LED_OFF;

}

void 
firemail_init(void) 
{
    uart_init();
    fm_btn_init();
    fm_led_init();

    STATE->stage = FM_INIT; STATE->sent = FM_INIT;
    STATE->processing = 0;

    stdout = &fmstdout;

    firemail_send_mail();

    debug_counter = 0;
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
  timer(100, firemail_periodic())
  init(firemail_init)
*/
