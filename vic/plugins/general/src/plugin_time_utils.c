/******************************************************************************
 * @section DESCRIPTION
 *
 * Functions for time support
 *
 * @section LICENSE
 *
 * The Variable Infiltration Capacity (VIC) macroscale hydrological model
 * Copyright (C) 2016 The Computational Hydrology Group, Department of Civil
 * and Environmental Engineering, University of Washington.
 *
 * The VIC model is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *****************************************************************************/

#include <vic_driver_image.h>
#include <plugin.h>

/******************************************************************************
 * @brief  returns the day in the defined season,
 *         or the number of days past the season
 *****************************************************************************/
double
between_dmy(dmy_struct start,
            dmy_struct end,
            dmy_struct current)
{
    return (between_jday((double)start.day_in_year + start.dayseconds /
                         SEC_PER_DAY,
                         (double)end.day_in_year + end.dayseconds / SEC_PER_DAY,
                         (double)current.day_in_year + current.dayseconds /
                         SEC_PER_DAY));
}

/******************************************************************************
 * @brief  returns the day in the defined season,
 *         or the number of days past the season
 *****************************************************************************/
double
between_jday(double start,
             double end,
             double current)
{
    if (start < end) {
        if (current > end) {
            return end - current;
        }
        else if (current > start) {
            return current - start;
        }
        else {
            return current - start;
        }
    }
    else {
        if (current > start) {
            return current - start;
        }
        else if (current > end) {
            return end - current;
        }
        else {
            return ((DAYS_PER_JYEAR - start) + current);
        }
    }
}

/******************************************************************************
 * @brief  returns the number of days in a month
 *****************************************************************************/
unsigned short int
days_per_month(unsigned short int month,
               unsigned short int year,
               unsigned short int calendar)
{
    unsigned short int days_per_month[MONTHS_PER_YEAR] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    days_per_month[1] += leap_year(year, calendar);

    return(days_per_month[month - 1]);
}
