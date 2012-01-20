/* Copyright (C) 2011
   Tony Mancill <tmancill@users.sourceforge.net>
   Dave Cinege <dcinege@psychosis.com>
   jos.dehaes@bigfoot.com

   This file is part of Rippix.

   Rippix is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   Rippix is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with Rippix.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include "common.h"

#define JC_START                    0
#define JC_UPDATE                   1
#define JC_PAUSE                    2
#define JC_CONT                     3
#define JC_ABORT                    4
#define JC_ABORT_DELETE             5
#define JC_ABORT_ALL                6
#define JC_ABORT_ALL_DELETE         7
#define JC_TIMEOUT                  450

void job_starter( _main_data *main_data );

void job_controller( int ops, _main_data *main_data );
/* Job controller. This function saves main_data
 * when called with JC_START and uses it for further operation */

#endif
