/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _ECMD_SERIAL_USART_H
#define _ECMD_SERIAL_USART_H

#define ECMD_SERIAL_USART_BUFFER_LEN 50
/* use 19200 baud  */
#define ECMD_SERIAL_BAUDRATE 19200

void ecmd_serial_usart_init(void);
void ecmd_serial_usart_periodic(void);

#endif /* _ECMD_SERIAL_USART_H */
