/*
 *
 * Copyright (c) 2009 by Jochen Roessner <jochen@lugrot.de>
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

#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "control6.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_c6_get(char *cmd, char *output, uint16_t len)
{
  uint8_t varvalue;

  if (control6_get(cmd, &varvalue))
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%s %u"), cmd, varvalue));
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_c6_set(char *cmd, char *output, uint16_t len)
{
  char *buf;
  uint8_t varvalue;
  buf = strrchr (cmd, ' ');
  if (buf) {
    *(buf ++) = 0;
    varvalue = atoi (buf);
    if (control6_set(cmd, varvalue))
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%s %u"), cmd, varvalue));
  }
    return ECMD_ERR_PARSE_ERROR;
}

