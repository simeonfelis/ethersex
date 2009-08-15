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

#include "firemail.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"

Testline

void 
firemail_process (void) 
{
static uip_conn_t *fm_connection = NULL;

	if (PINC & (1<<PC0))
	{
		PORTC &= ~(1<<PC1);
	}
	else
	{
		PORTC |= (1<<PC1);
		debug_printf("Button pressed");

	    /* make a connection */
	    uip_ipaddr_t fm_server;
	    uip_ipaddr(&fm_server, 192,168,178,25);
	
	    fm_connection = uip_connect(&fm_server, HTONS(25), fm_connection_established);
	}
}

void 
firemail_init(void) 
{
	DDRC &= ~(1<<PC0);
	DDRC |= (1<<PC1);
}

void
fm_connection_established(void)
{
    
}

/*
  -- Ethersex META --
  header(hardware/firemail/firemail.h)
  mainloop(firemail_process)
  init(firemail_init)
*/

