/*
 * File: time_current_tm_nano.c, author: John Sauter, date: December 5, 2016.
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

#include <sys/timex.h>

#include "time_subroutines.h"

/* Subroutine to return the current UTC time in a tm structure
 * and the number of nanoseconds since the start of the last second.  
 */
int
time_current_tm_nano (struct tm *current_tm, int *nanoseconds)
{
  struct timex current_timex;
  int adjtimex_result;

  /* Fetch time information from the kernel.  */
  current_timex.status = 0;
  current_timex.modes = 0;
  adjtimex_result = adjtimex (&current_timex);

  /* Format that information into a tm structure.  */
  gmtime_r (&current_timex.time.tv_sec, current_tm);

  /* If the kernel told us we are in a leap second, increment
   * the seconds value.  This will change it from 59 to 60.  */
  if (adjtimex_result == TIME_OOP)
    {
      current_tm->tm_sec = current_tm->tm_sec + 1;
    }

  /* Return the number of nanoseconds since the start of the last
   * second.  If the kernel's clock is not being controlled by
   * NTP it will return microseconds instead of nanoseconds.  */
  if (current_timex.status & STA_NANO)
    *nanoseconds = current_timex.time.tv_usec;
  else
    *nanoseconds = current_timex.time.tv_usec * 1e3;

  return (0);
}
