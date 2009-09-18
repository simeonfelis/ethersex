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
#include "protocols/dns/resolv.h"
#include "core/debug.h"

static uint8_t btn_state;
uip_conn_t *conn;

#if CONF_FM_DEBUG_LEVEL > 0
# define DEBUG_FIREMAIL
# if CONF_FM_DEBUG_LEVEL > 1
#  if CONF_FM_DEBUG_LEVEL > 2
#   warning "debug level too high. defaulting to 2"
#  endif
#  define DEBUG_FIREMAIL2
# endif
#endif /* CONF_FM_DEBUG_LEVEL */

#ifdef DEBUG_FIREMAIL
# define fmdebug(a...) debug_printf("fm: "a)
#else
# define fmdebug(a...)
#endif
#ifdef DEBUG_FIREMAIL2
# define fmdebug2(a...) debug_printf("fm: "a)
#else
# define fmdebug2(a...)
#endif

#define fm_send(a)                         \
do {                                       \
    memcpy_P(uip_appdata, a, sizeof(a));   \
    uip_send(uip_appdata, sizeof(a) -1);   \
} while (0);

static const char PROGMEM TXT_HELO[] = 
    "HELO "CONF_FM_HELO_NAME"\n";

static const char PROGMEM TXT_EHLO[] = 
    "EHLO "CONF_FM_HELO_NAME"\n";

static const char PROGMEM TXT_LOGIN[] = 
    "AUTH LOGIN\n";

static const char PROGMEM TXT_USER_64[] =
    CONF_FM_USER_64"\n";

static const char PROGMEM TXT_PASSWD_64[] = 
    CONF_FM_PASSWD_64"\n";

static const char PROGMEM TXT_MAIL[] =
    "MAIL FROM:"CONF_FM_FROM_ADDRESS"\n";

static const char PROGMEM TXT_RCPT[] = 
    "RCPT TO:"CONF_FM_TO_ADDRESS"\n";

static const char PROGMEM TXT_DATA[] = 
    "DATA\n";

static const char PROGMEM TXT_TRANSMIT[] = 
    "From: <feuer@feuerwehr.de>\n" 
    "To: <"CONF_FM_TO_ADDRESS">\n" 
    "Subject: Feuermeldung\n" 
    "\n" 
    "Hallo Feuerwehrmann,\n"
    "Das Feuermeldemodul hat einen Alarm erhalten. Bitte\n"
    "erfuellen Sie Ihre Bereitschaftspflicht\n"
    "\n"
    "Mit freundlichen Gruessen\n"
    "\n"
    "Ihre Feuerwehr\n" 
    "PS: Diese mail wurde vom AVR NETIO verschickt.\n"
    "    In den E-Mail Headern steht meine Adresse.\n"
    "    MFG Simeon Felis\n"
    ".\n";

static const char PROGMEM TXT_QUIT[] =
    "QUIT\n";

#define STATE (&uip_conn->appstate.firemail)
//struct firemail_connection_state_t *fms;

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
    fmdebug2("rec: %s\n", (char *)uip_appdata);
    switch (STATE->stage) {
    case FM_INIT:
        if (strstr_P (uip_appdata, PSTR("220")) == NULL) {
            fmdebug("rec: init failed: %s\n", (char *)uip_appdata);
            return 1;
        }
        break;
    case FM_EHLO:
    case FM_MAIL:
    case FM_RCPT:
    case FM_TRANSMIT:
        if (strstr_P (uip_appdata, PSTR("250")) == NULL) {
           fmdebug("rec: failed 250 stage %i: %s\n", STATE->stage, 
                                                 (char *)uip_appdata);
           return 1;  
        }
        break;
    case FM_LOGIN:
    case FM_USER:
        if (strstr_P (uip_appdata, PSTR("334")) == NULL) {
            fmdebug("rec: failed 334 stage %i: %s\n", STATE->stage, 
                                                  (char *)uip_appdata);
            return 1;
        }
        break;
    case FM_PASSWD:
        if (strstr_P (uip_appdata, PSTR("235")) == NULL) {
            fmdebug("rec: failed 235 stage %i: %s\n", STATE->stage,
                                                  (char *)uip_appdata);
            return 1;
        }
        break;
    case FM_DATA:
        if (strstr_P (uip_appdata, PSTR("354")) == NULL) {
            fmdebug("rec: failed 354 stage %i: %s\n", STATE->stage,
                                                       (char *)uip_appdata);
            return 1;
        }
        break;
    case FM_QUIT:
        if (strstr_P (uip_appdata, PSTR("221")) == NULL) {
            fmdebug("rec: QUIT failed stage %i: %s\n", STATE->stage,
                                                       (char *)uip_appdata);
            return 1;
        }
        break;
    default:
        fmdebug("rec: unsupported stage: %i\n", STATE->stage);
        return 1;
        break;
    }
    STATE->stage++;
    return 0;
}

void
firemail_send_data (uint8_t send_state)
{
    switch (send_state) {
    case FM_INIT:
        /* just connected to server. nothing to send */
        break;
    case FM_EHLO:
        fm_send(TXT_EHLO);
        break;
    case FM_LOGIN:
        fm_send(TXT_LOGIN);
        break;
    case FM_USER:
        fm_send(TXT_USER_64);
        break;
    case FM_PASSWD:
        fm_send(TXT_PASSWD_64);
        break;
    case FM_MAIL:
        fm_send(TXT_MAIL);
        break;
    case FM_RCPT:
        fm_send(TXT_RCPT);
        break;
    case FM_DATA:
        fm_send(TXT_DATA);
        break;
    case FM_TRANSMIT:
        fm_send(TXT_TRANSMIT);
        break;
    case FM_QUIT:
        fm_send(TXT_QUIT);
        break;
    case FM_FINISHED:
        break;
    default:
        fmdebug("send_data unsupported command: %i\n", send_state);
        return;
        break;
    }
    STATE->sent = send_state;
    fmdebug2("send stage: %i: %s\n", STATE->stage, (char *)uip_sappdata);
}

static void
firemail_query_cb(char *name, uip_ipaddr_t *server_ip)
{
    fmdebug2("query: to %s on %u %u\n",name, *server_ip[0], *server_ip[1]);

    conn = uip_connect(server_ip, HTONS(CONF_FM_SERVER_PORT), firemail_main);
    if (conn == NULL) {
        fmdebug("uip_conn fail lookup\n");
        return;
    }
    fmdebug2("serv found: %u %u %u %u\n",(*server_ip[0]&0x00FF), 
            (*server_ip[0]&0xFF00)>>8,
            (*server_ip[1]&0x00FF),
            (*server_ip[1]&0xFF00)>>8);
}

void
firemail_connect()
{
    fmdebug2("connect to "CONF_FM_SERVER_NAME" on %i...\n", CONF_FM_SERVER_PORT);
#ifdef DNS_SUPPORT
    uip_ipaddr_t *server_ip;
    if (!(server_ip = resolv_lookup(CONF_FM_SERVER_NAME))) {
        resolv_query(CONF_FM_SERVER_NAME, firemail_query_cb);
        fmdebug2("con: reslv started\n");
    }
    else {

        conn = uip_connect(server_ip, HTONS(CONF_FM_SERVER_PORT), firemail_main);
        if (conn == NULL) {
            fmdebug("uip_con fail\n");
            return;
        }
        fmdebug2("serv (intl): %u %u %u %u\n",(*server_ip[0]&0x00FF), 
                (*server_ip[0]&0xFF00)>>8,
                (*server_ip[1]&0x00FF),
                (*server_ip[1]&0xFF00)>>8);
    }
#else
# error "Firemail needs DNS_SUPPORT"
#endif /* DNS_SUPPORT */
}

void
firemail_main(void)
{
    fmdebug2("main: stage: %i sent: %i\n", STATE->stage, STATE->sent);
    if (uip_aborted()) {
        fmdebug("aborted\n");
        conn = NULL;
    }
    else if (uip_timedout()) {
        fmdebug("timeout\n");
        conn = NULL;
    }
    
    if (uip_closed()) {
        if (STATE->stage == FM_FINISHED && STATE->sent == FM_FINISHED) {
            STATE->processing = 0;
            fmdebug("Sendig mail done\n");
        }
        fmdebug("closed\n");
        conn = NULL;
    }

    if (uip_acked()) {
        fmdebug2("acked\n");
    }

    if (uip_newdata() && uip_len) {
        ((char *) uip_appdata)[uip_len] = 0;
        if (firemail_receive()) {
            fmdebug("received weired stuff\n");
            uip_close();
            return;
        }
    }

    if (uip_rexmit()) {
        fmdebug("resent\n");
        firemail_send_data(STATE->sent);
    }
    else if ((STATE->stage > STATE->sent) && 
             (uip_connected() || uip_acked() || uip_newdata())) {
        firemail_send_data(STATE->stage);
    }
    else if (uip_poll()) {
        /* we may do anything now. it's our turn. */
    }
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
    fm_btn_init();
    fm_led_init();

    STATE->stage = FM_INIT; STATE->sent = FM_INIT;
    STATE->processing = 0;

    firemail_send_mail();
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

