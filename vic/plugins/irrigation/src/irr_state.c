/******************************************************************************
 * @section DESCRIPTION
 *
 * Routing state functions
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

/******************************************
* @brief   Set state metadata
******************************************/
void
irr_set_state_meta_data_info(void)
{
    extern metadata_struct state_metadata[];

    snprintf(state_metadata[N_STATE_VARS + STATE_IRR_RECEIVED].varname,
             MAXSTRING, "%s", "STATE_IRR_RECEIVED");
    snprintf(state_metadata[N_STATE_VARS + STATE_IRR_RECEIVED].long_name,
             MAXSTRING, "%s", "received");
    snprintf(state_metadata[N_STATE_VARS + STATE_IRR_RECEIVED].standard_name,
             MAXSTRING, "%s", "received");
    snprintf(state_metadata[N_STATE_VARS + STATE_IRR_RECEIVED].units, MAXSTRING,
             "%s", "m3/s");
    snprintf(state_metadata[N_STATE_VARS + STATE_IRR_RECEIVED].description,
             MAXSTRING, "%s", "received amount of irrigation");
}

/******************************************
* @brief   Initialize state dimension size and ids
******************************************/
void
irr_set_nc_state_file_info(nc_file_struct *nc_state_file)
{

     //To do: fix below, adapt to irrigation
     
    extern global_param_struct        global_param;
    extern plugin_global_param_struct plugin_global_param;
    extern plugin_option_struct       plugin_options;

  //  nc_state_file->nc_id = MISSING;
    

}

/******************************************
* @brief   Add state dimensions
******************************************/
void
irr_add_state_dim(char           *filename,
                   nc_file_struct *nc_state_file)
{
    int status;

    // Define dimension
    /*
    status = nc_def_dim(nc_state_file->nc_id, "received",
                        nc_state_file->irr_size,
                        &(nc_state_file->irr_dimid));
    check_nc_status(status, "Error defining received irrigation in %s", filename);
    */
}

/******************************************
* @brief   Add state dimension variables
******************************************/
void
irr_add_state_dim_var(char           *filename,
                       nc_file_struct *nc_state_file)
{
    int dimids[MAXDIMS];
    int var_id;
    int status;

    // Initialize
 //   dimids[0] = nc_state_file->rdt_dimid;

    // Define dimension variable
    /*
    status = nc_def_var(nc_state_file->nc_id, "received",
                        NC_DOUBLE, 1, dimids, &(var_id));
    check_nc_status(status, "Error defining received variable in %s",
                    filename);
    status = nc_put_att_text(nc_state_file->nc_id, var_id,
                             "long_name",
                             strlen("received"), "received");
    check_nc_status(status, "Error adding attribute in %s", filename);
*/
}

/******************************************
* @brief   Add state dimension variable data
******************************************/
void
irr_add_state_dim_var_data(char           *filename,
                            nc_file_struct *nc_state_file)
{
// To do: Is the following necessary?
    /*
    size_t dstart[1];
    size_t dcount[1];
    int    var_id;
    int   *ivar;
    int    status;

    size_t i;

    // Initialize
    dstart[0] = 0;
    dcount[0] = nc_state_file->irr_size;

    status = nc_inq_varid(nc_state_file->nc_id,
                          "received", &var_id);
    check_nc_status(status, "Unable to find variable \"%s\" in %s",
                    "received",
                    filename);

    // Fill dimension variable
    ivar = malloc(nc_state_file->rdt_size * sizeof(*ivar));
    check_alloc_status(ivar, "Memory allocation error");

    status = nc_put_vara_int(nc_state_file->nc_id, var_id, dstart,
                             dcount, ivar);
    check_nc_status(status, "Error writing irrigation received id in %s",
                    filename);
*/
}

/******************************************
* @brief   Set state variable dimensions
******************************************/
void
irr_set_nc_state_var_info(nc_file_struct *nc,
                           size_t          varid)
{

    // Set the number of dimensions and dimids for each state variable
    switch (varid) {
    case N_STATE_VARS + STATE_IRR_RECEIVED:
            // 4d vars [band, irr, j, i]
            nc->nc_vars[varid].nc_dims = 4;
            nc->nc_vars[varid].nc_dimids[0] = nc->band_dimid;
            nc->nc_vars[varid].nc_dimids[1] = nc->irr_dimid;
            nc->nc_vars[varid].nc_dimids[2] = nc->nj_dimid;
            nc->nc_vars[varid].nc_dimids[3] = nc->ni_dimid;
            nc->nc_vars[varid].nc_counts[0] = 1;  // number of snow bands
            nc->nc_vars[varid].nc_counts[1] = 1;  // number of irrigation indices (assumed same for all cells)
            nc->nc_vars[varid].nc_counts[2] = nc->nj_size;  // number of rows
            nc->nc_vars[varid].nc_counts[3] = nc->ni_size;  // number of columns
        break;
    }

    
}

/******************************************
* @brief   Store states
******************************************/
void
irr_store(nc_file_struct *state_file)
{
    extern domain_struct              local_domain;
    extern global_param_struct        global_param;
    extern plugin_global_param_struct plugin_global_param;
    extern option_struct              options;
    extern irr_con_map_struct        *irr_con_map;
 
    irr_var_struct                   *cirr_var;
    size_t                            i;
    size_t                            iIrr;
    size_t                            iBand;
    size_t                            d4start[4];

    double                           *dvar = NULL;
    nc_var_struct                    *nc_var;
    int                               status;
 
    // write state variables

    // allocate memory for variables to be stored
    dvar = malloc(local_domain.ncells_active * sizeof(*dvar));
    check_alloc_status(dvar, "Memory allocation error");

    // initialize starts and counts
    d4start[0] = 0;
    d4start[1] = 0;
    d4start[2] = 0;
    d4start[3] = 0;

    // Adapted to irrigation 
    nc_var = &(state_file->nc_vars[N_STATE_VARS + STATE_IRR_RECEIVED]);


    FILE *file = fopen("received_dvar.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }


// Loop to save cirr_var values

for (i = 0; i < local_domain.ncells_active; i++) {
    for (iIrr = 0; iIrr < irr_con_map[i].ni_active; iIrr++) {
//        d4start[1] = iIrr;    // irrigation index

        for (iBand = 0; iBand < options.SNOW_BAND; iBand++) {
            cirr_var = &(irr_var[i][iIrr][iBand]);
 //           d4start[0] = iBand;   // snow band index

            // Save the received value to the state file
            dvar[i] = (double)cirr_var->received;
                        fprintf(file, "%f\n", dvar[i]);  // write dvar[i] to the file

        }

        // After looping through all iBand, save the values for current cell and irrigation index
        gather_put_nc_field_double(state_file->nc_id,
                                   nc_var->nc_varid,
                                   state_file->d_fillvalue,
                                   d4start, nc_var->nc_counts, dvar);

        // Optionally, reset dvar for debugging
        /*
        for (int i = 0; i < local_domain.ncells_active; i++) {
            dvar[i] = state_file->d_fillvalue;
        }*/
    }
}






    fclose(file);
    free(dvar);

}
