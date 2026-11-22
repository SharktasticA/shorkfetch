/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling the      ##
    ## computer's (host) name                           ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef HOST
#define HOST

#define HOST_LEN        256
#define VAL_BUFFER_LEN  128



#ifndef NO_STR_CLEANING
typedef enum 
{
    PF_THINKPAD
} ProductFamily;



struct TYPE_NAME {
    const char *type;
    const ProductFamily family;
    const char *name;
};
#endif



static const char *DMI_JUNK[] = {
    "To be filled by O.E.M.",
    "Not Available",
    "None",
    "No Asset Tag",
    "Default string"
};
static const int DMI_JUNK_LEN = sizeof(DMI_JUNK) / sizeof(DMI_JUNK[0]);

#ifndef NO_STR_CLEANING
// Hardcoded IBM type numbers to use in isIBMAndChanged(). We only record
// product series that include members post-DMI 2.0 (1998), which for
// ThinkPads, may provide the IBM type number but not the product name.
static const struct TYPE_NAME IBM_TYPES[] = {
    { "1161", PF_THINKPAD, "i Series 1200/1300" },
    { "1171", PF_THINKPAD, "130/i Series 1200/1300" },
    { "1829", PF_THINKPAD, "R50/R51" },
    { "1830", PF_THINKPAD, "R50/R51" },
    { "1831", PF_THINKPAD, "R50/R51" },
    { "1832", PF_THINKPAD, "R50p" },
    { "1833", PF_THINKPAD, "R50p" },
    { "1834", PF_THINKPAD, "R50e" },
    { "1836", PF_THINKPAD, "R50/R51" },
    { "1840", PF_THINKPAD, "R50/R51" },
    { "1841", PF_THINKPAD, "R50/R51" },
    { "1842", PF_THINKPAD, "R50e" },
    { "1843", PF_THINKPAD, "R51e" },
    { "1844", PF_THINKPAD, "R51e" },
    { "1845", PF_THINKPAD, "R51e" },
    { "1846", PF_THINKPAD, "R52" },
    { "1847", PF_THINKPAD, "R52" },
    { "1848", PF_THINKPAD, "R52" },
    { "1849", PF_THINKPAD, "R52" },
    { "1850", PF_THINKPAD, "R52" },
    { "1858", PF_THINKPAD, "R52" },
    { "1859", PF_THINKPAD, "R52" },
    { "1860", PF_THINKPAD, "R52" },
    { "1861", PF_THINKPAD, "R52" },
    { "1862", PF_THINKPAD, "R52" },
    { "1863", PF_THINKPAD, "R52" },
    { "1864", PF_THINKPAD, "X41" },
    { "1865", PF_THINKPAD, "X41" },
    { "1866", PF_THINKPAD, "X41 Tablet" },
    { "1867", PF_THINKPAD, "X41 Tablet" },
    { "1869", PF_THINKPAD, "X41 Tablet" },
    { "1870", PF_THINKPAD, "R52" },
    { "2366", PF_THINKPAD, "T30" },
    { "2367", PF_THINKPAD, "T30" },
    { "2369", PF_THINKPAD, "X40" },
    { "2370", PF_THINKPAD, "X40" },
    { "2371", PF_THINKPAD, "X40" },
    { "2372", PF_THINKPAD, "X40" },
    { "2373", PF_THINKPAD, "T40/T41/T42/p" },
    { "2374", PF_THINKPAD, "T40/T41/T42/p" },
    { "2375", PF_THINKPAD, "T40/T41/T42/p" },
    { "2376", PF_THINKPAD, "T40/T41/T42/p" },
    { "2378", PF_THINKPAD, "T40/T41/T42/p" },
    { "2379", PF_THINKPAD, "T40/T41/T42/p" },
    { "2382", PF_THINKPAD, "X40" },
    { "2384", PF_THINKPAD, "G40" },
    { "2386", PF_THINKPAD, "X40" },
    { "2387", PF_THINKPAD, "G40" },
    { "2388", PF_THINKPAD, "G40" },
    { "2389", PF_THINKPAD, "G40" },
    { "2525", PF_THINKPAD, "X41" },
    { "2526", PF_THINKPAD, "X41" },
    { "2527", PF_THINKPAD, "X41" },
    { "2528", PF_THINKPAD, "X41" },
    { "2607", PF_THINKPAD, "235" },
    { "2609", PF_THINKPAD, "240 Series/i Series 1124" },
    { "2611", PF_THINKPAD, "i Series 1400/1500" },
    { "2612", PF_THINKPAD, "240X" },
    { "2621", PF_THINKPAD, "i Series 1400/1500" },
    { "2624", PF_THINKPAD, "390X/i Series 1700" },
    { "2626", PF_THINKPAD, "390/390E/i Series 1700" },
    { "2627", PF_THINKPAD, "390/390X/i Series 1700" },
    { "2628", PF_THINKPAD, "A20m/A21m/A22m/i Series 1800" },
    { "2629", PF_THINKPAD, "A20p/A21p/A22p" },
    { "2632", PF_THINKPAD, "i Series 1800" },
    { "2633", PF_THINKPAD, "A20m/A21m/A22m" },
    { "2635", PF_THINKPAD, "380/385 Series" },
    { "2639", PF_THINKPAD, "i Series s30/s31" },
    { "2640", PF_THINKPAD, "560 Series" },
    { "2643", PF_THINKPAD, "570 Series" },
    { "2644", PF_THINKPAD, "570 Series/i Series 1157" },
    { "2645", PF_THINKPAD, "600 Series" },
    { "2646", PF_THINKPAD, "600X" },
    { "2647", PF_THINKPAD, "T20/T21/T22/T23" },
    { "2648", PF_THINKPAD, "T20/T21/T22/T23" },
    { "2651", PF_THINKPAD, "i Series 1400/1500" },
    { "2652", PF_THINKPAD, "A30/A31" },
    { "2653", PF_THINKPAD, "A30p/A31p" },
    { "2654", PF_THINKPAD, "A30/A31" },
    { "2655", PF_THINKPAD, "A21e/A22e/i Series 1800" },
    { "2656", PF_THINKPAD, "R30/R31" },
    { "2657", PF_THINKPAD, "R31" },
    { "2658", PF_THINKPAD, "R32" },
    { "2659", PF_THINKPAD, "R32" },
    { "2660", PF_THINKPAD, "X20/X21/X22/X23/X24" },
    { "2661", PF_THINKPAD, "X20/i Series 1620" },
    { "2662", PF_THINKPAD, "X20/X21/X22/X23/X24" },
    { "2662", PF_THINKPAD, "i Series 1620" },
    { "2663", PF_THINKPAD, "A22e" },
    { "2664", PF_THINKPAD, "A22e" },
    { "2668", PF_THINKPAD, "T43/T43p" },
    { "2669", PF_THINKPAD, "T43/T43p" },
    { "2670", PF_THINKPAD, "R50e" },
    { "2672", PF_THINKPAD, "X30/X31/X32" },
    { "2673", PF_THINKPAD, "X30/X31/X32" },
    { "2675", PF_THINKPAD, "TransNote" },
    { "2676", PF_THINKPAD, "R31" },
    { "2677", PF_THINKPAD, "R32" },
    { "2678", PF_THINKPAD, "T43/T43p" },
    { "2679", PF_THINKPAD, "T43/T43p" },
    { "2681", PF_THINKPAD, "R40" },
    { "2682", PF_THINKPAD, "R40" },
    { "2683", PF_THINKPAD, "R40" },
    { "2684", PF_THINKPAD, "R40e" },
    { "2685", PF_THINKPAD, "R40e" },
    { "2686", PF_THINKPAD, "T43/T43p" },
    { "2687", PF_THINKPAD, "T43/T43p" },
    { "2722", PF_THINKPAD, "R40" },
    { "2723", PF_THINKPAD, "R40" },
    { "2724", PF_THINKPAD, "R40" },
    { "2881", PF_THINKPAD, "G41" },
    { "2882", PF_THINKPAD, "G41" },
    { "2883", PF_THINKPAD, "R51" },
    { "2884", PF_THINKPAD, "X30/X31/X32" },
    { "2885", PF_THINKPAD, "X30/X31/X32" },
    { "2886", PF_THINKPAD, "G41" },
    { "2887", PF_THINKPAD, "R51" },
    { "2888", PF_THINKPAD, "R51" },
    { "2889", PF_THINKPAD, "R51" },
    { "2894", PF_THINKPAD, "R51" },
    { "2895", PF_THINKPAD, "R51" },
    { "2896", PF_THINKPAD, "R40" },
    { "2897", PF_THINKPAD, "R40" },
    { "2898", PF_THINKPAD, "R40" },
    { "2899", PF_THINKPAD, "R40" },
    { "9548", PF_THINKPAD, "770/770E/770ED" },
    { "9549", PF_THINKPAD, "770X/770Z" }
};
static const int IBM_TYPES_LEN = sizeof(IBM_TYPES) / sizeof(IBM_TYPES[0]);
#endif



char *getHost(void);
int inWSL(void);
int isDMIValJunk(const char*);
#ifndef NO_STR_CLEANING
int isIBMAndChanged(const char*, char*);
#endif

#endif
