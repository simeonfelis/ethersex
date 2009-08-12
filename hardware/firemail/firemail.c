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

#include "core/debug.h"

void 
firemail_process (void) 
{

	if (PINC && (1<<PC1))
	{
		debug_printf("Button Pressed\n");
	}

}

void 
firemail_init(void) 
{
	/* Set IO port as input */
	DDRC &= ~(1<<PC1);    
}

/*
  -- Ethersex META --
  header(hardware/firemail/firemail.h)
  mainloop(firemail_process)
  init(firemail_init)
*/

