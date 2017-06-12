/*
 * File: time_tm_nano_to_integer.c, author: John Sauter, 
 * date: January 9, 2017.   */

/*
 * Copyright © 2017 by John Sauter <John_Sauter@systemeyescomputerstore.com>

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

#include "time_subroutines.h"

/* Subroutine to convert a time in tm format to an integer.
 * Unlike time_t, this integer has a unique value during a 
 * leap second.  time_t counts seconds except during a 
 * leap second.  This integer never counts seconds, and 
 * includes fractions of a second.  
 */
int
time_tm_nano_to_integer (struct tm *input_tm, int input_nanoseconds,
			 __int128 *result)
{
  int year, month, day, hour, minute, second;
  __int128 result_value;
  
  year = input_tm->tm_year + 1900;
  month = input_tm->tm_mon + 1;
  day = input_tm->tm_mday;
  hour = input_tm->tm_hour;
  minute = input_tm->tm_min;
  second = input_tm->tm_sec;

  result_value = year;
  result_value = (result_value * 1e2) + month;
  result_value = (result_value * 1e2) + day;
  result_value = (result_value * 1e2) + hour;
  result_value = (result_value * 1e2) + minute;
  result_value = (result_value * 1e2) + second;
  result_value = (result_value * (__int128) 1e9) +
    (__int128) input_nanoseconds;
  *result = result_value;

  return (0);
}
