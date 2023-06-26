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
dam_check_init_state_file(void)
{
    extern filenames_struct           filenames;
    extern plugin_option_struct       plugin_options;
    extern int                        *dam_classes;
    extern int                        mpi_rank;
    extern global_param_struct        global_param;

    int                               state_ndams;
    int                               *state_dam_classes;
    size_t                            j;  
    size_t                            d1start[1];
    size_t                            d1count[1];
    dmy_struct                        start_dmy;
    dmy_struct                        init_dmy;

    if (mpi_rank == VIC_MPI_ROOT){
        state_ndams = get_nc_dimension(&(filenames.init_state), "dam_class");
        if(plugin_options.NDAMTYPES != state_ndams){
            log_err("dam class size in state file does not match dam_class size "
                    "in dam paramater file");
        }

        state_dam_classes = malloc(state_ndams * sizeof(*state_dam_classes));
        check_alloc_status(state_dam_classes, "Memory allocation error");

        d1start[0] = 0;
        d1count[0] = plugin_options.NDAMTYPES;

        get_nc_field_int(&(filenames.init_state),"dam_class",d1start,d1count,state_dam_classes);
        for ( j=0; j < plugin_options.NDAMTYPES; j++){
            if(state_dam_classes[j] != dam_classes[j]){
                log_err("dam_clas values in state file does not match "
                    "dam_class values in dam param file.")
            }
        }

        // Restart can only be done from the when start of simulations equals the init state timestamp
        start_dmy.dayseconds = global_param.startsec;
        start_dmy.year = global_param.startyear;
        start_dmy.day = global_param.startday;
        start_dmy.month = global_param.startmonth;

        init_dmy.dayseconds = global_param.initsec;
        init_dmy.year = global_param.inityear;
        init_dmy.day = global_param.initday;
        init_dmy.month = global_param.initmonth;


        if(!dmy_equal(&start_dmy, &init_dmy)){
            log_err("Cannot restore dams when the INIT_STATE time is not equal to simulation start time.");
        }

    }
}


/******************************************
* @brief   Restore states
******************************************/
void
dam_restore(void)
{
    extern filenames_struct     filenames;
    extern metadata_struct      state_metadata[];
    extern domain_struct        local_domain;
    extern dam_con_map_struct   *dam_con_map;
    extern dam_var_struct       **dam_var;

    size_t                      i;
    size_t                      j;
    int                         dam_index;
    size_t                      d1count[1];
    size_t                      d1start[1];
    double                      *dvar = NULL;
    int                         *ivar = NULL;

    d1count[0] = 1;
    dvar = malloc(1 * sizeof(*dvar));
    check_alloc_status(dvar, "Memory allocation error");
    ivar = malloc(1 * sizeof(*ivar));
    check_alloc_status(ivar, "Memory allocation error");

    for (j = 0; j < plugin_options.NDAMTYPES; j++){
        for (i = 0; i < local_domain.ncells_active; i++){
            dam_index = dam_con_map[i].didx[j];
            if(dam_index != NODATA_DAM){
                d1start[0] = j;                
                /// restore the storage
                get_nc_field_double(&(filenames.init_state),state_metadata[N_STATE_VARS + STATE_DAM_STORAGE].varname,
                    d1start,d1count,dvar);
                dam_var[i][dam_index].storage = dvar[0];
                // restore the inflow 
                get_nc_field_double(&(filenames.init_state),state_metadata[N_STATE_VARS + STATE_DAM_INFLOW].varname,
                    d1start,d1count,dvar);
                dam_var[i][dam_index].total_inflow = dvar[0];
                // restore total demand
                get_nc_field_double(&(filenames.init_state),state_metadata[N_STATE_VARS + STATE_DAM_DEMAND].varname,
                    d1start,d1count,dvar);
                dam_var[i][dam_index].total_demand = dvar[0];
                // restore total efr
                get_nc_field_double(&(filenames.init_state),state_metadata[N_STATE_VARS + STATE_DAM_EFR].varname,
                    d1start,d1count,dvar);
                dam_var[i][dam_index].total_efr = dvar[0];
                // restore months
                get_nc_field_int(&(filenames.init_state),state_metadata[N_STATE_VARS + STATE_DAM_MONTHS].varname,
                    d1start,d1count,ivar);
                dam_var[i][dam_index].months_running = ivar[0];
                // restore registered steps
                get_nc_field_int(&(filenames.init_state),state_metadata[N_STATE_VARS + STATE_DAM_STEPS].varname,
                    d1start,d1count,ivar);
                dam_var[i][dam_index].register_steps = ivar[0];
            }
        }
    }
    
    free(dvar);
    free(ivar);
}

/******************************************
* @brief   Calculate derived state variables
******************************************/
void dam_compute_derived_state_vars(void)
{
    extern domain_struct        local_domain;
    extern dam_con_map_struct   *dam_con_map;
    extern dam_var_struct       **dam_var;

    size_t iDam;
    size_t i;

    for (i = 0; i < local_domain.ncells_active; i++){

        for (iDam = 0; iDam < dam_con_map[i].nd_active; iDam++){
            if(dam_var[i][iDam].months_running > 0){
                dam_var[i][iDam].active = true;
            }
        }
    }
    
}