/******************************************************************************
* @section DESCRIPTION
*
* This subroutine controls the model core, it solves both the energy and water
* balance models, as well as frozen soils.
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
******************************************************************************/

#include <vic_run.h>

veg_lib_struct *vic_run_veg_lib;

/******************************************************************************
* @brief        This subroutine controls the model core, it solves both the
*               energy and water balance models, as well as frozen soils.
******************************************************************************/
int
vic_run(force_data_struct   *force,
        all_vars_struct     *all_vars,
        dmy_struct          *dmy,
        global_param_struct *gp,
        lake_con_struct     *lake_con,
        soil_con_struct     *soil_con,
        veg_con_struct      *veg_con,
        veg_lib_struct      *veg_lib)
{
    extern option_struct     options;
    extern parameters_struct param;

    char                     overstory;
    size_t                   lidx;
    unsigned short           iveg;
    size_t                   Nveg;
    unsigned short           veg_class;
    unsigned short           band;
    size_t                   Nbands;
    int                      ErrorFlag;
    double                   out_prec[MAX_BANDS];
    double                   out_rain[MAX_BANDS];
    double                   out_snow[MAX_BANDS];
    double                   dp;
    double                   ice0[MAX_BANDS];
    double                   moist0[MAX_BANDS];
    double                   surf_atten;
    double                   wind_h;
    double                   height;
    double                   displacement[3];
    double                   roughness[3];
    double                   ref_height[3];
    double                   aero_resist[3];
    double                   Cv;
    double                   Le;
    double                   Melt[MAX_BANDS];
    double                   bare_albedo;
    double                   snow_inflow[MAX_BANDS];
    double                   rainonly;
    double                   sum_runoff;
    double                   sum_baseflow;
    double                   tmp_wind[3];
    double                   gauge_correction[2];
    double                   lag_one;
    double                   sigma_slope;
    double                   fetch;
    double                   lakefrac;
    double                   fraci;
    double                   wetland_runoff;
    double                   wetland_baseflow;
    double                   snowprec;
    double                   rainprec;
    size_t                   cidx;
    lake_var_struct         *lake_var;
    cell_data_struct       **cell;
    veg_var_struct         **veg_var;
    energy_bal_struct      **energy;
    snow_data_struct       **snow;

    // assign vic_run_veg_lib to veg_lib, so that the veg_lib for the correct
    // grid cell is used within vic_run. For simplicity sake, use vic_run_veg_lib
    // everywhere within vic_run
    vic_run_veg_lib = veg_lib;

    /* set local pointers */
    cell = all_vars->cell;
    energy = all_vars->energy;
    lake_var = &all_vars->lake_var;
    snow = all_vars->snow;
    veg_var = all_vars->veg_var;

    Nbands = options.SNOW_BAND;

    /* Set number of vegetation tiles */
    Nveg = veg_con[0].vegetat_type_num;

    /** Set Damping Depth **/
    dp = soil_con->dp;

    /* Compute gauge undercatch correction factors
       - this assumes that the gauge is free of vegetation effects, so gauge
       correction is constant for the entire grid cell */
    if (options.CORRPREC && force->prec[NR] > 0) {
        correct_precip(gauge_correction, force->wind[NR], gp->wind_h,
                       soil_con->rough, soil_con->snow_rough);
    }
    else {
        gauge_correction[0] = 1;
        gauge_correction[1] = 1;
    }
    force->out_prec = 0;
    force->out_rain = 0;
    force->out_snow = 0;

    /** initialize moist0 and ice0 **/
    // this could be removed if we make them scalars
    for (band = 0; band < options.SNOW_BAND; band++) {
        moist0[band] = 0;
        ice0[band] = 0;
    }

    /**************************************************
       Solve Energy and/or Water Balance for Each
       Vegetation Tile
    **************************************************/
    for (iveg = 0; iveg <= Nveg; iveg++) {
        /** Solve Veg Tile only if Coverage Greater than 0% **/
        if (veg_con[iveg].Cv > 0.0) {
            Cv = veg_con[iveg].Cv;
            Nbands = options.SNOW_BAND;

            /** Define vegetation class number **/
            veg_class = veg_con[iveg].veg_class;

            /**************************************************
               Initialize Model Parameters
            **************************************************/

            /** Lake-specific processing **/
            if (veg_con[iveg].LAKE) {
                /* Update areai to equal new ice area from previous time step. */
                lake_var->areai = lake_var->new_ice_area;

                /* Compute lake fraction and ice-covered fraction */
                if (lake_var->areai < 0) {
                    lake_var->areai = 0;
                }
                if (lake_var->sarea > 0) {
                    fraci = lake_var->areai / lake_var->sarea;
                    if (fraci > 1.0) {
                        fraci = 1.0;
                    }
                }
                else {
                    fraci = 0.0;
                }
                lakefrac = lake_var->sarea / lake_con->basin[0];

                Nbands = 1;
                Cv *= (1 - lakefrac);

                if (Cv == 0) {
                    continue;
                }
            }

            /** Assign wind_h **/
            /** Note: this is ignored below **/
            wind_h = vic_run_veg_lib[veg_class].wind_h;

            /* Initialize wind speeds */
            tmp_wind[0] = force->wind[NR];
            tmp_wind[1] = MISSING;
            tmp_wind[2] = MISSING;

            /* Set surface descriptive variables */
            displacement[0] = veg_var[iveg][0].displacement;
            roughness[0] = veg_var[iveg][0].roughness;
            if (roughness[0] == 0) {
                roughness[0] = soil_con->rough;
            }
            overstory = vic_run_veg_lib[veg_class].overstory;

            /* Estimate vegetation height */
            height = calc_veg_height(displacement[0]);

            /* Estimate reference height */
            if (displacement[0] < wind_h) {
                ref_height[0] = wind_h;
            }
            else {
                ref_height[0] = displacement[0] + wind_h + roughness[0];
            }

            /* Compute aerodynamic resistance */
            ErrorFlag = CalcAerodynamic(overstory, height,
                                        vic_run_veg_lib[veg_class].trunk_ratio,
                                        soil_con->snow_rough, soil_con->rough,
                                        vic_run_veg_lib[veg_class].wind_atten,
                                        aero_resist, tmp_wind,
                                        displacement, ref_height,
                                        roughness);
            if (ErrorFlag == ERROR) {
                return (ERROR);
            }

            /** Compute Surface Attenuation due to Vegetation Coverage **/
            surf_atten = (1 - veg_var[iveg][0].fcanopy) * 1.0 +
                         veg_var[iveg][0].fcanopy *
                         exp(-vic_run_veg_lib[veg_class].rad_atten *
                             veg_var[iveg][0].LAI);

            /** Compute Bare (free of snow) Albedo **/
            if (iveg != Nveg) {
                bare_albedo = veg_var[iveg][0].albedo;
            }
            else {
                bare_albedo = param.ALBEDO_BARE_SOIL;
            }

            /**************************************************
               Loop over elevation bands
            **************************************************/
            for (band = 0; band < Nbands; band++) {
                /** Solve band only if coverage greater than 0% **/
                if (soil_con->AreaFract[band] > 0) {
                    /******************************************
                       Initialize Band-dependent Model Parameters
                    ******************************************/

                    /* Initialize soil thermal properties for the top two layers */
                    prepare_full_energy(&(cell[iveg][band]),
                                        &(energy[iveg][band]),
                                        soil_con, &(moist0[band]),
                                        &(ice0[band]));

                    /* Initialize final aerodynamic resistance values */
                    cell[iveg][band].aero_resist[0] =
                        aero_resist[0];
                    cell[iveg][band].aero_resist[1] =
                        aero_resist[1];

                    /* Initialize pot_evap */
                    cell[iveg][band].pot_evap = 0;

                    // Convert LAI from global to local
                    veg_var[iveg][band].LAI /= veg_var[iveg][band].fcanopy;
                    veg_var[iveg][band].Wdew /= veg_var[iveg][band].fcanopy;
                    veg_var[iveg][band].Wdmax = veg_var[iveg][band].LAI *
                                                param.VEG_LAI_WATER_FACTOR;
                    snow[iveg][band].snow_canopy /= veg_var[iveg][band].fcanopy;

                    /** Initialize other veg vars **/
                    if (iveg < Nveg) {
                        veg_var[iveg][band].rc = param.HUGE_RESIST;

                        /* Carbon-related variables */
                        if (options.CARBON) {
                            for (cidx = 0; cidx < options.Ncanopy; cidx++) {
                                veg_var[iveg][band].rsLayer[cidx] =
                                    param.HUGE_RESIST;
                            }
                            veg_var[iveg][band].aPAR = 0;

                            calc_Nscale_factors(
                                vic_run_veg_lib[veg_class].NscaleFlag,
                                veg_con[iveg].CanopLayerBnd,
                                veg_var[iveg][band].LAI,
                                force->coszen[NR],
                                veg_var[iveg][band].NscaleFactor);

                            // TBD: move this outside of vic_run()
                            if (dmy->day_in_year == 1) {
                                veg_var[iveg][band].AnnualNPPPrev =
                                    veg_var[iveg][band].AnnualNPP;
                                veg_var[iveg][band].AnnualNPP = 0;
                            }
                        } // if options.CARBON
                    } // if iveg < Nveg

                    /* Initialize energy balance variables */
                    energy[iveg][band].shortwave = 0;
                    energy[iveg][band].longwave = 0.;

                    /* Initialize snow variables */
                    snow[iveg][band].vapor_flux = 0.;
                    snow[iveg][band].canopy_vapor_flux = 0.;
                    snow_inflow[band] = 0.;
                    Melt[band] = 0.;

                    /* Initialize precipitation storage */
                    out_prec[band] = 0;
                    out_rain[band] = 0;
                    out_snow[band] = 0;

                    /******************************
                       Solve ground surface fluxes
                    ******************************/

                    lag_one = veg_con[iveg].lag_one;
                    sigma_slope = veg_con[iveg].sigma_slope;
                    fetch = veg_con[iveg].fetch;

                    ErrorFlag = surface_fluxes(overstory, bare_albedo,
                                               ice0[band], moist0[band],
                                               surf_atten, &(Melt[band]),
                                               &Le,
                                               aero_resist,
                                               displacement, gauge_correction,
                                               &out_prec[band],
                                               &out_rain[band],
                                               &out_snow[band],
                                               ref_height, roughness,
                                               &snow_inflow[band],
                                               tmp_wind, veg_con[iveg].root,
                                               options.Nlayer, Nveg, band, dp,
                                               iveg, veg_class, force, dmy,
                                               &(energy[iveg][band]), gp,
                                               &(cell[iveg][band]),
                                               &(snow[iveg][band]),
                                               soil_con, &(veg_var[iveg][band]),
                                               lag_one, sigma_slope, fetch,
                                               veg_con[iveg].CanopLayerBnd);

                    if (ErrorFlag == ERROR) {
                        return (ERROR);
                    }

                    force->out_prec +=
                        out_prec[band] * Cv * soil_con->AreaFract[band];
                    force->out_rain +=
                        out_rain[band] * Cv * soil_con->AreaFract[band];
                    force->out_snow +=
                        out_snow[band] * Cv * soil_con->AreaFract[band];

                    /********************************************************
                       Compute soil wetness and root zone soil moisture
                    ********************************************************/
                    cell[iveg][band].rootmoist = 0;
                    cell[iveg][band].wetness = 0;
                    for (lidx = 0; lidx < options.Nlayer; lidx++) {
                        if (veg_con[iveg].root[lidx] > 0) {
                            cell[iveg][band].rootmoist +=
                                cell[iveg][band].layer[lidx].moist;
                        }
                        cell[iveg][band].wetness +=
                            (cell[iveg][band].layer[lidx].moist -
                             soil_con->Wpwp[lidx]) /
                            (soil_con->porosity[lidx] * soil_con->depth[lidx] *
                             MM_PER_M - soil_con->Wpwp[lidx]);
                    }
                    cell[iveg][band].wetness /= options.Nlayer;

                    /* Convert LAI back to global */
                    veg_var[iveg][band].LAI *= veg_var[iveg][band].fcanopy;
                    veg_var[iveg][band].Wdmax *= veg_var[iveg][band].fcanopy;
                } /** End non-zero area band **/
            } /** End Loop Through Elevation Bands **/
        } /** end non-zero area veg tile **/
    } /** end of vegetation loop **/

    // Compute gridcell-averaged albedo
    calc_gridcell_avg_albedo(&all_vars->gridcell_avg.avg_albedo,
                             force->shortwave[NR], Nveg, energy,
                             veg_con, soil_con);

    /****************************
       Run Lake Model
    ****************************/

    /** Compute total runoff and baseflow for all vegetation types
        within each snowband. **/
    if (options.LAKES && lake_con->lake_idx >= 0) {
        wetland_runoff = wetland_baseflow = 0;
        sum_runoff = sum_baseflow = 0;

        // Loop through all vegetation tiles
        for (iveg = 0; iveg <= Nveg; iveg++) {
            /** Solve Veg Tile only if Coverage Greater than 0% **/
            if (veg_con[iveg].Cv > 0.) {
                Cv = veg_con[iveg].Cv;
                Nbands = options.SNOW_BAND;
                if (veg_con[iveg].LAKE) {
                    Cv *= (1 - lakefrac);
                    Nbands = 1;
                }

                // Loop through snow elevation bands
                for (band = 0; band < Nbands; band++) {
                    if (soil_con->AreaFract[band] > 0) {
                        if (veg_con[iveg].LAKE) {
                            wetland_runoff += (cell[iveg][band].runoff *
                                               Cv * soil_con->AreaFract[band]);
                            wetland_baseflow += (cell[iveg][band].baseflow *
                                                 Cv *
                                                 soil_con->AreaFract[band]);
                            cell[iveg][band].runoff = 0;
                            cell[iveg][band].baseflow = 0;
                        }
                        else {
                            sum_runoff += (cell[iveg][band].runoff *
                                           Cv * soil_con->AreaFract[band]);
                            sum_baseflow += (cell[iveg][band].baseflow *
                                             Cv * soil_con->AreaFract[band]);
                            cell[iveg][band].runoff *= (1 - lake_con->rpercent);
                            cell[iveg][band].baseflow *=
                                (1 - lake_con->rpercent);
                        }
                    }
                }
            }
        }

        /** Run lake model **/
        iveg = lake_con->lake_idx;
        band = 0;
        lake_var->runoff_in =
            (sum_runoff * lake_con->rpercent +
             wetland_runoff) * soil_con->cell_area / MM_PER_M;                                               // m3
        lake_var->baseflow_in =
            (sum_baseflow * lake_con->rpercent +
             wetland_baseflow) * soil_con->cell_area / MM_PER_M;                                                 // m3
        lake_var->channel_in = force->channel_in[NR] * soil_con->cell_area /
                               MM_PER_M;                                        // m3
        lake_var->prec = force->prec[NR] * lake_var->sarea / MM_PER_M; // m3
        rainonly = calc_rainonly(force->air_temp[NR], force->prec[NR],
                                 param.SNOW_MAX_SNOW_TEMP,
                                 param.SNOW_MIN_RAIN_TEMP);
        if ((int) rainonly == ERROR) {
            return(ERROR);
        }

        /**********************************************************************
           Solve the energy budget for the lake.
        **********************************************************************/

        snowprec = gauge_correction[SNOW] * (force->prec[NR] - rainonly);
        rainprec = gauge_correction[SNOW] * rainonly;
        Cv = veg_con[iveg].Cv * lakefrac;
        force->out_prec += (snowprec + rainprec) * Cv;
        force->out_rain += rainprec * Cv;
        force->out_snow += snowprec * Cv;

        ErrorFlag = solve_lake(snowprec, rainprec, force->air_temp[NR],
                               force->wind[NR], force->vp[NR] / PA_PER_KPA,
                               force->shortwave[NR], force->longwave[NR],
                               force->vpd[NR] / PA_PER_KPA,
                               force->pressure[NR] / PA_PER_KPA,
                               force->density[NR], lake_var,
                               *soil_con, gp->dt, gp->wind_h, *dmy,
                               fraci);
        if (ErrorFlag == ERROR) {
            return (ERROR);
        }

        /**********************************************************************
           Solve the water budget for the lake.
        **********************************************************************/

        ErrorFlag = water_balance(lake_var, *lake_con, gp->dt, all_vars,
                                  iveg, band, lakefrac, *soil_con,
                                  veg_con[iveg]);
        if (ErrorFlag == ERROR) {
            return (ERROR);
        }
    } // end if (options.LAKES && lake_con->lake_idx >= 0)

    return (0);
}
