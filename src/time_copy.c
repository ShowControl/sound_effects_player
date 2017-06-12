/*
 * File: time_copy.c, author: John Sauter, date: December 5, 2016.
 */

/*
 * Copyright © 2016 by John Sauter <John_Sauter@systemeyescomputerstore.com>

 * This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *  The author's contact information is as follows:
 *    John Sauter
 *    System Eyes Computer Store
 *    20A Northwest Blvd.  Ste 345
 *    Nashua, NH  03063-4066
 *    telephone: (603) 424-1188
 *    e-mail: John_Sauter@systemeyescomputerstore.com
 */

#include <string.h>

#include "time_subroutines.h"

/* Copy a time from one tm structure to another.  */
int
time_copy_tm (struct tm *in_tm, struct tm *out_tm)
{
  memset (out_tm, 0, sizeof (*out_tm));
  out_tm->tm_year = in_tm->tm_year;
  out_tm->tm_mon = in_tm->tm_mon;
  out_tm->tm_mday = in_tm->tm_mday;
  out_tm->tm_hour = in_tm->tm_hour;
  out_tm->tm_min = in_tm->tm_min;
  out_tm->tm_sec = in_tm->tm_sec;
  out_tm->tm_yday = in_tm->tm_yday;
  out_tm->tm_wday = in_tm->tm_wday;
  out_tm->tm_isdst = in_tm->tm_isdst;
  out_tm->tm_gmtoff = in_tm->tm_gmtoff;
  out_tm->tm_zone = in_tm->tm_zone;
  
  return (0);
}
