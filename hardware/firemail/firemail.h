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

#ifndef _FIREMAIL_H
#define _FIREMAIL_H

void firemail_process(void);
void firemail_init(void);

void fm_conn_estab(void);
void uart_init(void);
void fm_btn_poll(void);



#define FM_LED_ACTIVATE         (DDRC |= (1<<PC1))
#define FM_LED_ON               (PORTC &= ~(1<<PC1))
#define FM_LED_OFF              (PORTC |= (1<<PC1))

#define FM_BTN_ACTIVATE         (DDRC &= ~(1<<PC0))
#define FM_IS_BTN_PUSHED        (PINC & (1<<PC0))

#define BAUD    1152
#define UART_BAUD_RATE 9600

#ifdef USE_2X
#warning "USE_2X was defined, now undefined"
#undef USE_2X
#endif

#endif /* _FIREMAIL_H */

