/*
	Redqueen Chess Engine
    Copyright (C) 2008-2009 Ben-Hur Carlos Vieira Langoni Junior

    This file is part of Redqueen Chess Engine.

    Redqueen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Redqueen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Redqueen.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Inline.cpp
 *
 *  Created on: Apr 8, 2009
 *      Author: bhlangonijr
 */
#include <inttypes.h>
#include <iostream>
#define MIN(x,y) x<y?x:y;

#if !defined(_MSC_VER)

#  if defined(_SC_NPROCESSORS_ONLN)
int getNumProcs() {
  return MIN(sysconf( _SC_NPROCESSORS_ONLN ), 8);
}
#  else
int getNumProcs() {
  return 1;
}
#  endif

#else

int getNumProcs() {
  SYSTEM_INFO s;
  GetSystemInfo(&s);
  return MIN(s.dwNumberOfProcessors, 8);
}

#endif
