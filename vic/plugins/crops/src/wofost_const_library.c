#include <vic_def.h>

float ScatCoef = 0.2;
float XGauss[] = {
    0.1127017, 0.5000000, 0.8872983
};
float WGauss[] = {
    0.2777778, 0.4444444, 0.2777778
};

char *ManageParam[MAXSTRING] = {
    "NRFTAB",
    "PRFTAB",
    "KRFTAB",
    "NMINS",
    "RTNMINS",
    "PMINS",
    "RTPMINS",
    "KMINS",
    "RTKMINS",
    "NULL"
};
char *ManageParam2[MAXSTRING] = {
    "FERNTAB",
    "FERPTAB",
    "FERKTAB",
    "IRRTAB",
    "NULL"
};

char *CropParam[MAXSTRING] = {
    "TBASEM",
    "TEFFMX",
    "TSUMEM",
    "IDSL",
    "DLO",
    "DLC",
    "VERNSAT",
    "VERNBASE",
    "TSUM1",
    "TSUM2",
    "DVSI",
    "DVSEND",
    "TDWI",
    "RGRLAI",
    "SPA",
    "SPAN",
    "TBASE",
    "CVL",
    "CVO",
    "CVR",
    "CVS",
    "Q10",
    "RML",
    "RMO",
    "RMR",
    "RMS",
    "PERDL",
    "CFET",
    "DEPNR",
    "IAIRDU",
    "RDI",
    "RRI",
    "RDMCR",
    "RDRLV_NPK",
    "DVS_NPK_STOP",
    "DVS_NPK_TRANSL",
    "NPK_TRANSLRT_FR",
    "NCRIT_FR",
    "PCRIT_FR",
    "KCRIT_FR",
    "NMAXRT_FR",
    "NMAXST_FR",
    "PMAXRT_FR",
    "PMAXST_FR",
    "KMAXRT_FR",
    "KMAXST_FR",
    "NLAI_NPK",
    "NLUE_NPK",
    "NMAXSO",
    "PMAXSO",
    "KMAXSO",
    "NPART",
    "NSLA_NPK",
    "NRESIDLV",
    "NRESIDST",
    "NRESIDRT",
    "PRESIDLV",
    "PRESIDST",
    "PRESIDRT",
    "KRESIDLV",
    "KRESIDST",
    "KRESIDRT",
    "TCNT",
    "TCPT",
    "TCKT",
    "NFIX_FR",
    "MaxHeight",
    "RGL",
    "RadAtten",
    "WindAtten",
    "TrunkRatio",
    "Albedo",
    "MinStomResist",
    "MaxArchResist",
    "Fcanopy",
    "FRTRL",
    "CRITLAI",
    "NULL"
};
char *CropParam2[MAXSTRING] = {
    "VERNRTB",
    "DTSMTB",
    "SLATB",
    "SSATB",
    "KDIFTB",
    "EFFTB",
    "AMAXTB",
    "TMPFTB",
    "TMNFTB",
    "CO2AMAXTB",
    "CO2EFFTB",
    "CO2TRATB",
    "RFSETB",
    "FRTB",
    "FLTB",
    "FSTB",
    "FOTB",
    "RDRRTB",
    "RDRSTB",
    "NMAXLV_TB",
    "PMAXLV_TB",
    "KMAXLV_TB",
    "NULL"
};
