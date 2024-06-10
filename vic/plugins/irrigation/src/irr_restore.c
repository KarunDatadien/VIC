/******************************************************************************
 * @section DESCRIPTION
 *
 * Dam restore functions
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

#include <vic_driver_shared_image.h>
#include <plugin.h>

/******************************************
* @brief   Check state file
******************************************/
void
irr_check_init_state_file(void)
{
  // To do: add check
}

/******************************************
* @brief   Restore states
******************************************/
void
irr_restore(void)
{
  // to do: add declarations and modify code below for irrigation:
/*
    for (j = 0; j < plugin_options.IRRIGATION; j++) {
        for (i = 0; i < local_domain.ncells_active; i++) {
            dam_index = dam_con_map[i].didx[j];
            if (dam_index != NODATA_DAM) {
                d1start[0] = j;
                d2start[1] = j;
                /// restore the received irrigation
                get_nc_field_double(&(filenames.init_state),
                                    state_metadata[N_STATE_VARS +
                                                   STATE_IRR_RECEIVED].varname,
                                    d1start, d1count, dvar);
                dam_var[i][dam_index].storage = dvar[0];
              
            }
        }
    }

    free(dvar);
    free(ivar);
    free(d2monvar);
    free(d2histvar);
*/
}

/******************************************
* @brief   Calculate derived state variables
******************************************/
void
irr_compute_derived_state_vars(void)
{
/*
    // To do: adapt below part for irrigation. Even necessary?
    extern domain_struct       local_domain;
    extern dam_con_map_struct *dam_con_map;
    extern dam_var_struct    **dam_var;

    size_t                     iDam;
    size_t                     i;

    for (i = 0; i < local_domain.ncells_active; i++) {
        for (iDam = 0; iDam < dam_con_map[i].nd_active; iDam++) {
            if (dam_var[i][iDam].months_running > 0) {
                dam_var[i][iDam].active = true;
            }
        }
    }
*/
}
