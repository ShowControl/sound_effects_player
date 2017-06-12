/*
 * File: time_sleep_until.c, author: John Sauter, date: January 14, 2017.
 */

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

/* Sleep until a specified time.  */
int
time_sleep_until (struct tm *time_tm, int nanoseconds,
		  int variable_length_seconds_before_1972)
{
  long long int seconds_to_sleep;
  struct tm now_tm;
  int now_nanoseconds, ns_to_sleep;
  struct timespec request_timespec;
  struct timespec rem_timespec;
  int return_value;
  
  /* Fetch the current time.  */
  time_current_tm_nano (&now_tm, &now_nanoseconds);

  /* Compute the number of seconds until the target time.  */
  seconds_to_sleep =
    time_diff (&now_tm, time_tm,
	       variable_length_seconds_before_1972);

  /* Adjust for the number of nanoseconds that have passed since
   * the last second, and the number that should pass after the
   * target second is reached before we awaken.  */
  ns_to_sleep = nanoseconds - now_nanoseconds;
  if (ns_to_sleep < 0)
    {
      ns_to_sleep = ns_to_sleep + 1e9;
      seconds_to_sleep = seconds_to_sleep - 1;
    }

  /* If the target time has already arrived, don't sleep.  */
  if (seconds_to_sleep < 0)
    return (0);
  if ((seconds_to_sleep == 0) && (ns_to_sleep <= 0))
    return (0);
 
  request_timespec.tv_sec = seconds_to_sleep;
  request_timespec.tv_nsec = ns_to_sleep;

  return_value = nanosleep (&request_timespec, &rem_timespec);
  return (return_value);
}
