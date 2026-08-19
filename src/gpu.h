/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling GPUs/    ##
    ## graphics cards                                   ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef GPU
#define GPU



#define GPU_NAME_LEN    256
#define MAX_GPUS        4



typedef struct {
    char *name;
    int vendor;
    int device;
    int revision;
} GPU_IDS;



#ifndef NO_STR_CLEANING

extern const char *INTEL_IGPUS[];

#endif



char *cleanGPUName(const char*, const char*, const int);
GPU_IDS* getGPUs(int *);
char *interpretGPU(GPU_IDS*, const char *);

#endif
