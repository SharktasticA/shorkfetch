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



#include "exclusions.h"
#include "general.h"
#include "globals.h"
#include "gpu.h"
#ifndef NO_STR_CLEANING
#include "replacements.h"
#endif

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#ifndef NO_STR_CLEANING

// Harcoded Intel iGPU names to use via device ID lookup to replace "messy"
// data from pci.ids
const char *INTEL_IGPUS[] = {
//  DID:       Name:                                       Example host CPUs:
    [0x0042] = "HD Graphics (Clarkdale)",               // Core i5-650
    [0x0046] = "HD Graphics (Arrandale)",               // Core i3-380M
    //[004a] = "UNKNOWN Clarkdale/Arrandale?",          // 
    [0x0102] = "HD Graphics 2000",                      // Core i5-2500
    [0x0106] = "HD Graphics (Sandy Bridge)",            // Pentium B980
    [0x010a] = "HD Graphics P3000",                     // Xeon E3-1245 
    //[010e] = "UNKNOWN",                               //  
    [0x0112] = "HD Graphics 3000",                      // Core i3-2125
    [0x0116] = "HD Graphics 3000",                      // Core i3-2310M
    [0x0122] = "HD Graphics 3000",                      // Core i7-2600K
    [0x0126] = "HD Graphics 3000",                      // Core i5-2450M/i5-2520M
    [0x0152] = "HD Graphics 2500",                      // Core i5-3470
    [0x0162] = "HD Graphics 4000",                      // Core i7-3770
    [0x0166] = "HD Graphics 4000",                      // Core i5-3380M
    [0x016a] = "HD Graphics P4000",                     // Xeon E3-1245 v2
    [0x0402] = "HD Graphics for 4th Gen",               // Pentium G3220/G3240
    [0x0406] = "HD Graphics 5500",                      // Core i5-5200U
    [0x040a] = "HD Graphics for 4th Gen",               // Xeon E3-1265L v3
    [0x0412] = "HD Graphics 4600",                      // Core i5-4690K
    [0x0416] = "HD Graphics 4600",                      // Core i7-4720HQ
    [0x041a] = "HD Graphics P4600",                     // Xeon E3-1285 v3
    [0x041e] = "HD Graphics 4600",                      // Core i7-4790K
    [0x0a06] = "HD Graphics for 4th Gen",               // Celeron 2955U
    [0x0a16] = "HD Graphics 4400",                      // Core i7-4600U
    //[0a22] = "UNKNOWN Haswell ULT GT3 desktop",       // 
    [0x0a26] = "HD Graphics 5000",                      // Core i5-4250U
    [0x0a2a] = "HD Graphics 5500",                      // Core i7-5600U
    [0x0a2e] = "Iris Graphics 5100",                    // Core i5-4258U
    //[0d02] = "UNKNOWN Crystal Well GT1 desktop",      // 
    //[0d06] = "UNKNOWN Crystal Well GT1 mobile",       // 
    //[0d0a] = "UNKNOWN Crystal Well GT1 server",       // 
    //[0d0b] = "UNKNOWN Crystal Well GT1 workstation",  // 
    //[0d0e] = "UNKNOWN Crystal Well GT1 desktop",      // 
    //[0d12] = "UNKNOWN Crystal Well GT2 desktop",      // 
    //[0d16] = "UNKNOWN Crystal Well GT2 mobile",       // 
    //[0d1a] = "UNKNOWN Crystal Well GT2 server",       // 
    //[0d1b] = "UNKNOWN Crystal Well GT2 workstation",  // 
    //[0d1e] = "UNKNOWN Crystal Well GT2 desktop",      // 
    //[0d22] = "UNKNOWN Crystal Well GT3 desktop",      // 
    //[0d26] = "UNKNOWN Crystal Well GT3 mobile",       // 
    //[0d2a] = "UNKNOWN Crystal Well GT3 server",       // 
    //[0d2b] = "UNKNOWN Crystal Well GT3 workstation",  // 
    //[0d2e] = "UNKNOWN Crystal Well GT3 desktop",      // 
    //[0d36] = "UNKNOWN Crystal Well",                  // 
    //[1602] = "UNKNOWN Broadwell-U",                   // 
    [0x1606] = "HD Graphics for 5th Gen",               // Celeron 3205U
    //[160a] = "UNKNOWN Broadwell-U",                   // 
    //[160b] = "UNKNOWN Broadwell-U",                   // 
    //[160d] = "UNKNOWN Broadwell-U",                   // 
    //[160e] = "UNKNOWN Broadwell-U",                   // 
    [0x1612] = "HD Graphics 5600",                      // Core i7-5700HQ
    [0x1616] = "HD Graphics 5500",                      // Core i3-5005U
    //[161a] = "UNKNOWN Broadwell-U",                   // 
    //[161b] = "UNKNOWN Broadwell-U",                   // 
    //[161d] = "UNKNOWN Broadwell-U",                   // 
    [0x161e] = "HD Graphics 5300",                      // Core M-5Y51
    [0x1622] = "Iris Pro Graphics 6200",                // Core i5-5675C
    [0x1626] = "HD Graphics 6000",                      // Core i5-5350U
    [0x162a] = "Iris Pro Graphics P6300",               // Xeon E3-1285 v4
    [0x162b] = "Iris Graphics 6100",                    // Core i5-5257U
    //[162d] = "UNKNOWN Broadwell-U",                   // 
    //[162e] = "UNKNOWN Broadwell-U",                   // 
    //[1632] = "UNKNOWN Broadwell-U",                   // 
    //[1636] = "UNKNOWN Broadwell-U",                   // 
    //[163a] = "UNKNOWN Broadwell-U",                   // 
    //[163b] = "UNKNOWN Broadwell-U",                   // 
    //[163d] = "UNKNOWN Broadwell-U",                   // 
    //[163e] = "UNKNOWN Broadwell-U",                   // 
    [0x1902] = "HD Graphics 510",                       // Celeron G3920
    [0x1906] = "HD Graphics 510",                       // Celeron 3955U
    [0x190b] = "HD Graphics 510",                       // 
    [0x1912] = "HD Graphics 530",                       // Core i5-6500
    [0x1916] = "HD Graphics 520",                       // Core i7-6500U
    [0x191b] = "HD Graphics 530",                       // Core i7-6700HQ
    [0x191d] = "HD Graphics P530",                      // Xeon E3-1535M v5
    [0x191e] = "HD Graphics 515",                       // Core m3-6Y30/m5-6Y57
    [0x1921] = "HD Graphics 520",                       // Core i3-6006U
    [0x1923] = "HD Graphics 535",                       // 
    [0x1926] = "Iris Graphics 540",                     // Core i7-6560U
    [0x1927] = "Iris Graphics 550",                     // Core i5-6267U
    [0x192b] = "Iris Graphics 555",                     // 
    [0x192d] = "Iris Graphics P555",                    // 
    [0x1932] = "Iris Pro Graphics 580",                 // 
    [0x193a] = "Iris Pro Graphics P580",                // Xeon E3-1585 v5
    [0x193b] = "Iris Pro Graphics 580",                 // Core i7-6770HQ
    [0x193d] = "Iris Pro Graphics P580",                // Xeon E3-1545M v5
    [0x22b0] = "HD Graphics (Cherry Trail)",            // Atom x5-Z8300
    [0x22b1] = "HD Graphics 400/405",                   // Celeron N3060, Pentium N3700/N3710

    [0x2562] = "Extreme Graphics (845)",                // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://www.intel.com/content/www/us/en/support/products/81530/graphics/legacy-graphics/graphics-drivers-for-intel-82845g-graphics-controller.html
    [0x2572] = "Extreme Graphics 2 (865G)",             // https://theretroweb.com/chips/2808

    [0x2582] = "GMA 900 (910GL/915G/GV)",               // https://theretroweb.com/chips/3001
    [0x2592] = "GMA 900 (910GML/915GM/GME/GMS)",        // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://www.intel.com/content/dam/doc/datasheet/mobile-915-910-express-chipset-datasheet.pdf
    [0x258a] = "GMA 900 (E7221)",                       // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://www.intel.de/content/dam/doc/datasheet/e7221-chipset-datasheet.pdf
    [0x2772] = "GMA 950 (945G/GC/GZ)",                  // https://www.intel.com/Assets/PDF/datasheet/307502.pdf
    //[2776] = "",                                      // EXCLUDED - 380 for 2772
    //[2782] = "",                                      // EXCLUDED - 380 for 2582
    //[2792] = "",                                      // EXCLUDED - 380 for 2592
    [0x27a2] = "GMA 950 (940GML/943GML/945GM/GMS)",     // https://www.thinkwiki.org/wiki/Intel_Graphics_Media_Accelerator_950
    //[27a6] = "",                                      // EXCLUDED - 380 for 27ae
    [0x27ae] = "GMA 950 (945GME/GSE)",                  // https://groups.google.com/g/fa.linux.kernel/c/L82W9WHaeVk, https://www.notebookcheck.net/Review-Asus-Eee-PC-901-Netbook.11994.0.html
    [0x2972] = "GMA X3000 (946GZ)",                     // https://www.intel.com/Assets/PDF/datasheet/313083.pdf, https://theretroweb.com/motherboards/s/asus-p5b-mx
    //[2973] = "",                                      // EXCLUDED - 380 for 2972
    [0x2982] = "GMA X3500 (G35)",                       // https://www.intel.com/Assets/PDF/datasheet/317607.pdf, https://www.intel.com/Assets/PDF/prodbrief/317363.pdf
    //[2983] = "",                                      // EXCLUDED - 380 for 2982
    [0x2992] = "GMA 3000 (Q963/Q965)",                  // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://www.intel.com/Assets/PDF/datasheet/313053.pdf
    //[2993] = "",                                      // EXCLUDED - 380 for 2992
    [0x29a2] = "GMA X3000 (G965)",                      // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://theretroweb.com/motherboard/manual/d4681803-en-66cf8783c1307002457174.pdf
    //[29a3] = "",                                      // EXCLUDED - 380 for 29a2
    [0x29b2] = "GMA 3100 (Q35)",                        // https://ubuntu-mate.community/t/intel-graphic-card-optiplex-killed-with-an-update/28920/7, https://dl.dell.com/manuals/all-products/esuprt_desktop/esuprt_optiplex_desktop/optiplex-755_user%27s%20guide_en-us.pdf
    //[29b3] = "",                                      // EXCLUDED - 380 for 29b2
    [0x29c2] = "GMA 3100 (G31/G33)",                    // https://www.intel.com/Assets/PDF/datasheet/316966.pdf, https://forums.linuxmint.com/viewtopic.php?t=349169, https://theretroweb.com/motherboards/s/gigabyte-ga-g31m-es2l-rev-1-x
    //[29c3] = "",                                      // EXCLUDED - 380 for 29c2
    [0x29d2] = "GMA 3100 (Q33)",                        // https://forums.linuxmint.com/viewtopic.php?t=357510, https://www.bargainhardware.co.uk/content/specifications/DC5800_datasheet.pdf
    //[29d3] = "",                                      // EXCLUDED - 380 for 29d2
    [0x2a02] = "GMA X3100 (GL960/GM965)",               // https://www.intel.com/Assets/PDF/datasheet/316273.pdf
    //[2a03] = "",                                      // EXCLUDED - 380 for 2a02
    [0x2a12] = "GMA X3100 (GLE960/GME965)",             // https://www.intel.com/Assets/PDF/datasheet/316273.pdf
    //[2a13] = "",                                      // EXCLUDED - 380 for 2a13

    [0x2a42] = "GMA 4500MHD (GL40/GM4x/GS4x)",          // https://www.thinkwiki.org/wiki/Intel_GMA_4500MHD, https://www.intel.com/content/dam/doc/product-brief/mobile-gm45-intel-gs45-gl40-express-chipsets-brief.pdf
    //[2a43] = "",                                      // EXCLUDED - 380 for 2a42
    [0x2e02] = "GMA X4500 (G41/G43)",                   // https://en.wikipedia.org/w/index.php?title=Intel_GMA&oldid=282579198
    //[2e03] = "",                                      // EXCLUDED - 380 for 2e02
    [0x2e12] = "GMA 4500 (Q43/Q45)",                    // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units
    //[2e13] = "",                                      // EXCLUDED - 380 for 2e12
    [0x2e22] = "GMA X4500/X4500HD (G43/G45)",           // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units
    //[2e23] = "",                                      // EXCLUDED - 380 for 2e22
    [0x2e32] = "GMA X4500 (G41)",                       // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://forums.linuxmint.com/viewtopic.php?t=420549, https://i.dell.com/sites/csdocuments/Business_solutions_engineering-Docs_Documents/en/optiplex-380-tech-guide.pdf
    //[2e33] = "",                                      // EXCLUDED - 380 for 2e32
    [0x2e42] = "GMA 4500 (B43)",                        // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://forums.linuxmint.com/viewtopic.php?t=328843, https://images10.newegg.com/UploadFilesForNewegg/itemintelligence/HP/40001520003538489.pdf
    //[2e43] = "",                                      // EXCLUDED - 380 for 2e42
    [0x2e92] = "GMA 4500 (B43)",                        // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://lists.ubuntu.com/archives/kernel-bugs/2010-September/141165.html
    //[2e93] = "",                                      // EXCLUDED - 380 for 2e92

    [0x3184] = "UHD Graphics 605",                      // Pentium Silver J5005
    [0x3185] = "UHD Graphics 600",                      // Celeron N4020

    [0x3577] = "Extreme Graphics (830M/MG)",            // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units, https://megatokyo.moe/dl/msn_tv_2/datasheets/Intel_82830mg_NOT_82830M_GMCH-M.pdf
    [0x3582] = "Extreme Graphics 2 (852/855GM)",        // https://www.intel.com/content/dam/doc/datasheet/852gm-852gmv-chipset-graphics-memory-controller-hub-datasheet.pdf, https://theretroweb.com/chip/documentation/30326901-65c7fd89bd07c733983117.pdf, https://www.thinkwiki.org/wiki/Intel_Extreme_Graphics_2
    [0x358e] = "Extreme Graphics 2 (854)",              // https://theretroweb.com/chips/2821, https://theretroweb.com/chip/documentation/d1534303-65be5ae55062e765790772.pdf, https://theretroweb.com/chip/documentation/30682502-65be5ae54d457579437108.pdf

    [0x3e90] = "UHD Graphics 610",                      // Pentium Gold G5400
    [0x3e91] = "UHD Graphics 630",                      // Core i3-8100
    [0x3e92] = "UHD Graphics 630",                      // Core i5-8400T/i5-8500/i5-9500T/i7-8700K
    [0x3e93] = "UHD Graphics 610",                      // Celeron G4900
    [0x3e94] = "UHD Graphics P630",                     // Xeon E-2176M/E-2186M
    [0x3e96] = "UHD Graphics P630",                     // Xeon E-2224G/E-2146G/E-2276G
    [0x3e98] = "UHD Graphics 630",                      // Core i7-9700K
    [0x3e9a] = "UHD Graphics P630",                     // Xeon E-2278G
    [0x3e9b] = "UHD Graphics 630",                      // Core i7-8750H/i7-9750H
    [0x3e9c] = "UHD Graphics 610",                      // 
    [0x3ea0] = "UHD Graphics 620",                      // Core i5-8265U
    [0x3ea1] = "UHD Graphics 620",                      // Celeron 4205U/Pentium Gold 5405U
    [0x3ea5] = "Iris Plus Graphics 655",                // Core i5-8259U
    [0x3ea6] = "Iris Plus Graphics 645",                // Core i5-8257U
    [0x3ea8] = "Iris Plus Graphics 655",                // 
    [0x3ea9] = "UHD Graphics 620",                      // Core i5-8260U
    [0x4555] = "UHD Graphics for 10th Gen (16EU)",      // Celeron J6413/N6211
    [0x4571] = "UHD Graphics for 10th Gen (32EU)",      // Atom x6425E, Pentium J6426
    [0x4626] = "Iris Xe Graphics",                      // Core i7-12700H
    [0x4628] = "Iris Xe Graphics",                      // Core i5-1235U
    //[462a] = "UNKNOWN Alder Lake-P",                  // 
    //[4636] = "UNKNOWN Alder Lake-P",                  // 
    //[4638] = "UNKNOWN Alder Lake-P",                  // 
    //[463a] = "UNKNOWN Alder Lake-P",                  // 
    [0x4680] = "UHD Graphics 770",                      // Core i5-13500/i7-12700K
    [0x4682] = "UHD Graphics 730",                      // Core i5-12400/i5-13400
    [0x4688] = "UHD Graphics 770",                      // Core i7-13700HX
    //[468a] = "UNKNOWN Alder Lake-S",                  // 
    [0x468b] = "UHD Graphics 710",                      // Core i5-12450HX/i5-13450HX
    [0x4690] = "UHD Graphics 770",                      // Core i5-12600
    [0x4692] = "UHD Graphics 730",                      // Core i5-12400
    [0x4693] = "UHD Graphics 710",                      // Celeron G6900, Pentium Gold G7400
    //[46a0] = "UNKNOWN Alder Lake-P",                  //             
    //[46a1] = "UNKNOWN",                               // 
    [0x4c8a] = "UHD Graphics 750",                      // Core i7-11700K
    [0x4c8b] = "UHD Graphics 730",                      // Core i5-11400
    [0x4c90] = "UHD Graphics P750",                     // Xeon W-1350P/W-1370/W-1370P
    [0x4c9a] = "UHD Graphics P750",                     // Xeon E-2356G/E-2388G
    [0x4e55] = "UHD Graphics (Jasper Lake, 16EU)",      // Celeron N4500/N5095
    [0x4e61] = "UHD Graphics (Jasper Lake, 24EU)",      // Celeron N5105
    [0x4e71] = "UHD Graphics (Jasper Lake, 32EU)",      // Pentium Silver N6000/N6005
    [0x5902] = "HD Graphics 610",                       // Celeron G3930, Pentium G4560
    [0x5906] = "HD Graphics 610",                       // Celeron 3867U, Pentium Gold 4415U/4417U
    [0x590b] = "HD Graphics 610",                       // 
    [0x5912] = "HD Graphics 630",                       // Core i5-7500
    [0x5916] = "HD Graphics 620",                       // Core i7-7600U
    [0x5917] = "UHD Graphics 620",                      // Core i5-8250U
    [0x591b] = "HD Graphics 630",                       // Core i7-7700HQ
    [0x591c] = "UHD Graphics 615",                      // Core m3-8100Y
    [0x591d] = "HD Graphics P630",                      // Xeon E3-1245 v6/E3-1225 v6
    [0x591e] = "HD Graphics 615",                       // Celeron 3965Y, Core i5-7Y57
    [0x5921] = "HD Graphics 620",                       // Core i3-7020U
    [0x5923] = "HD Graphics 635",                       // 
    [0x5926] = "Iris Plus Graphics 640",                // Core i5-7360U
    [0x5927] = "Iris Plus Graphics 650",                // Core i7-7567U
    [0x5a84] = "HD Graphics 505",                       // Pentium J4205/N4200
    [0x5a85] = "HD Graphics 500",                       // Celeron N3350/N3450
    [0x87c0] = "UHD Graphics 617",                      // Core i5-8210Y
    [0x87ca] = "UHD Graphics 617",                      // 
    //[8a51] = "Iris Plus Graphics G7 (Ice Lake)",      // 
    //[8a52] = "Iris Plus Graphics G7",                 // 
    //[8a53] = "Iris Plus Graphics G7",                 // 
    //[8a56] = "Iris Plus Graphics G1 (Ice Lake)",      // 
    //[8a58] = "Ice Lake-Y GT1 [UHD Graphics G1]",      // 
    //[8a5a] = "Iris Plus Graphics G4 (Ice Lake)",      // 
    //[8a5c] = "Iris Plus Graphics G4 (Ice Lake)",      // 
    //[9840] = "Lakefield GT2 [UHD Graphics]",          // 
    //[9841] = "Lakefield GT1.5 [UHD Graphics]",        // 
    //[9a40] = "Tiger Lake-UP4 GT2 [Iris Xe Graphics]", // 
    [0x9a49] = "Iris Xe Graphics (80EU)",               // Core i5-1135G7
    //[9a60] = "TigerLake-H GT1 [UHD Graphics]",        // 
    //[9a68] = "TigerLake-H GT1 [UHD Graphics]",        // 
    //[9a70] = "Tiger Lake-H GT1 [UHD Graphics]",       // 
    [0x9a78] = "UHD Graphics for 11th Gen (48EU)",      // Core i3-1115G4
    [0x9b21] = "UHD Graphics 610",                      // Celeron 5305U, Pentium Gold 6405U
    [0x9b41] = "UHD Graphics for 10th Gen (24EU)",      // Core i7-10510U/i7-10610U
    [0x9ba4] = "UHD Graphics 610",                      // Core i5-10200H
    [0x9ba8] = "UHD Graphics 610",                      // Celeron G5905, Pentium Gold G6400
    [0x9baa] = "UHD Graphics 610",                      // Celeron 5205U
    //[9bac] = "Comet Lake UHD Graphics",               // 
    [0x9bc4] = "UHD Graphics 630",                      // Core i5-10300H
    [0x9bc5] = "UHD Graphics 630",                      // Core i7-10700/i7-10700K
    [0x9bc6] = "UHD Graphics P630",                     // Xeon W-1250P/W-1290P
    [0x9bc8] = "UHD Graphics 630",                      // Core i3-10100/i5-10500
    [0x9bca] = "UHD Graphics for 10th Gen (24EU)",      // Core i5-10210U/i7-10710U
    [0x9bcc] = "UHD Graphics for 10th Gen (24EU)",      // Core i5-10210U/i7-10510U/i7-10610U/i7-10710U
    [0x9be6] = "UHD Graphics P630",                     // Xeon W-1250
    [0x9bf6] = "UHD Graphics P630",                     // Xeon W-10885M
    [0xa720] = "Iris Xe Graphics (80-96EU)",            // Core i5-13500H/i9-13900H
    [0xa721] = "Iris Xe Graphics (80-96EU)",            // Core i5-1335U/i7-1355U
    [0xa780] = "UHD Graphics 770",                      // Core i7-13700K/i9-14900T
    //[a781] = "Raptor Lake-S UHD Graphics",            // 
    [0xa782] = "UHD Graphics 730",                      // Core i5-13400
    //[a783] = "Raptor Lake-S UHD Graphics",            // 
    [0xa788] = "UHD Graphics 770",                      // Core i9-14900HX
    //[a789] = "Raptor Lake-S UHD Graphics",            // 
    //[a78a] = "Raptor Lake-S UHD Graphics",            // 
    [0xa78b] = "UHD Graphics 710",                      // Core i5-13450HX/i7-13650HX
    [0xa7a0] = "Iris Xe Graphics (80EU)",               // Core i5-13500H
    [0xa7a1] = "Iris Xe Graphics (80EU)",               // Core i5-1335U
    [0xa7a8] = "UHD Graphics 770",                      // Core i7-13620H
    [0xa7a9] = "UHD Graphics for 13th Gen (64EU)",      // Core i3-1315U
    [0xa7aa] = "Graphics (48-96EU)",                    // Core 5 210H/7 250H/9 270H
    [0xa7ab] = "Graphics (48-64EU)",                    // Core 5 210H/7 240H
    [0xa7ac] = "Graphics (80-96EU)",                    // Core 5 120U/7 150U
    [0xa7ad] = "Graphics (64EU)"                        // Core 3 100U
};



/**
 * Cleans a GPU's name so it is less needlessly verbose and 'to the point'.
 * @param vendor Input vendor name string
 * @param device Input device name string
 * @param isGPUFromCPU Flags if the GPU name being cleaned is one extracted
 *                     from a CPU name
 * @return String containing the result after cleaning
 */
char *cleanGPUName(const char *vendor, const char *device,
    const int isGPUFromCPU)
{
    if (!vendor || !device) return strdup("");

    // Prepare result strings
    const int RESULT_SIZE = (GPU_NAME_LEN * 2) + 1;
    char *result = malloc(RESULT_SIZE);
    if (!result) return strdup("");
    char *cleanedVendor = NULL;
    char *cleanedDevice = strdup(device);
    char *cleanedDeviceNorm = NULL;
    char *cleanedDeviceBrac = NULL;



    // Shorten " / " to "/"
    if (strstr(cleanedDevice, " / "))
    {
        char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN, " / ", "/");
        free(cleanedDevice);
        cleanedDevice = tmp;
    }

    // If we find an opening square bracket, break up device name into
    // "normal" and "bracket" strings
    const char *brac = strchr(cleanedDevice, '[');
    if (brac)
    {
        int normLen = (int)(brac - cleanedDevice);
        cleanedDeviceNorm = strndup(cleanedDevice, normLen);

        // Remove trailing space
        if (normLen > 0 && cleanedDeviceNorm[normLen - 1] == ' ')
            cleanedDeviceNorm[normLen - 1] = '\0';

        // Find closing bracket
        const char *end = strchr(brac + 1, ']');
        if (end)
        {
            int bracLen = (int)(end - (brac + 1));
            cleanedDeviceBrac = strndup(brac + 1, bracLen);
        }
        // If no closing bracket, we treat this as invalid...
        else cleanedDeviceBrac = NULL;
    }
    else
    {
        cleanedDeviceNorm = strdup(device);
        cleanedDeviceBrac = NULL;
    }



    // Vendor-specific actions...
    // Advanced Micro Devices, Inc. [AMD/ATI]
    if (vendor[0] == 'A' && (strncmp(vendor, "AMD", 3) == 0 ||
        strncmp(vendor, "Advanced Micro", 14) == 0))
    {
        // If we used amdgpu.ids, A deterimed "AMD" or "ATI" may be in the
        // device name and we should use that
        if (strncmp(cleanedDevice, "AMD ", 4) == 0)
        {
            cleanedVendor = strdup("AMD");
            memmove(cleanedDevice, cleanedDevice + 4,
                strlen(cleanedDevice) - 3);
        }
        else if (strncmp(cleanedDevice, "ATI ", 4) == 0)
        {
            cleanedVendor = strdup("ATI");
            memmove(cleanedDevice, cleanedDevice + 4,
                strlen(cleanedDevice) - 3);
        }
        else
            cleanedVendor = strdup("AMD/ATI");

        // If we have bracketed info, we *may* discard the norm (usually
        // just containing the codename)
        if (cleanedDeviceBrac)
        {
            // If the info contains the "Graphics" (e.g., "Radeon R6
            // Graphics") or "Vega Series", we will permit the norm to help
            // with distinguishing it
            if (strstr(cleanedDeviceBrac, " Graphics") ||
                strstr(cleanedDeviceBrac, " Vega Series"))
                snprintf(cleanedDevice, GPU_NAME_LEN, "%s (%s)",
                    cleanedDeviceBrac, cleanedDeviceNorm);
            // Otherwise, we just use the bracketed info
            else
                snprintf(cleanedDevice, GPU_NAME_LEN, "%s",
                    cleanedDeviceBrac);
        }

        // Begin testing for if there are multiple "Radeon"... Look for the
        // first instance of "Radeon"
        char *first = strstr(cleanedDevice, "Radeon");
        if (first)
        {
            // Handle " Radeon"; +6 so we don't trip on the first instance
            char *next = first + 6;
            while ((next = strstr(next, " Radeon")))
                memmove(next, next + 7, strlen(next + 7) + 1);

            // Handle "Radeon "
            next = first + 6;
            while ((next = strstr(next, "Radeon ")))
                memmove(next, next + 7, strlen(next + 7) + 1);
        }

        // Especially for Vega iGPU strings like "Radeon Vega Series /
        // Radeon Vega Mobile Series"
        if (strstr(cleanedDevice, " Series/Vega Mobile Series"))
        {
            char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN,
                " Series/Vega Mobile Series", "/Vega Mobile");
            free(cleanedDevice);
            cleanedDevice = tmp;
        }

        // Prettify (e.g.) "FirePro V (FireGL V)" to "FirePro V/FireGL V"
        char *fireGLBrac = strstr(cleanedDevice, " (Fire");
        if (fireGLBrac)
        {
            char *closeBrac = strchr(fireGLBrac, ')');
            if (closeBrac)
            {
                memmove(fireGLBrac, fireGLBrac + 1, strlen(fireGLBrac));
                cleanedDevice[fireGLBrac - cleanedDevice] = '/';
                char *newClose = strchr(cleanedDevice, ')');
                if (newClose)
                    memmove(newClose, newClose + 1, strlen(newClose));
            }
        }
    }
    // Intel Corporation
    else if (vendor[0] == 'I' && strncmp(vendor, "Intel", 5) == 0)
    {
        cleanedVendor = strdup("Intel");

        // If we have bracketed info, we discard the norm (usually just
        // containing the core name)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);

        // Some Arc GPUs have "Intel" in the device name...
        if (strncmp(cleanedDevice, "Intel ", 6) == 0)
            memmove(cleanedDevice, cleanedDevice + 6,
                strlen(cleanedDevice) - 5);
    }
    // NVIDIA Corporation
    else if (vendor[0] == 'N' && strncmp(vendor, "NVIDIA", 6) == 0)
    {
        cleanedVendor = strdup("NVIDIA");

        // If we have bracketed info, we discard the norm (usually just
        // containing the core name)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);
    }
    // 3Dfx Interactive, Inc.
    else if (vendor[0] == '3' && strncmp(vendor, "3Dfx", 4) == 0)
    {
        cleanedVendor = strdup("3Dfx");

        // If this Voodoo is probably a Velocity (entry-level)
        if (cleanedDeviceBrac && cleanedDeviceBrac[0] == 'V')
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);

        // E.g., Voodoo 4/Voodoo 5 -> Voodoo 4/5
        if (strstr(cleanedDevice, "/Voodoo "))
        {
            char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN, "/Voodoo ",
                "/");
            free(cleanedDevice);
            cleanedDevice = tmp;
        }
    }
    else if (vendor[0] == 'C')
    {
        // Chips and Technologies
        if (vendor[1] == '&' || strncmp(vendor, "Chips and", 9) == 0)
        {
            if (COMPACT)
                cleanedVendor = strdup("C&T");
            else
                cleanedVendor = strdup("Chips & Technologies");
        }
        // Cirrus Logic
        else if (strncmp(vendor, "Cirrus Logic", 12) == 0)
        {
            cleanedVendor = strdup("Cirrus Logic");

            // Discard any bracketed info like "Alpine" 
            if (cleanedDeviceBrac)
                snprintf(cleanedDevice, GPU_NAME_LEN, "%s",
                    cleanedDeviceNorm);

            // Remove space between "GD" and model number
            if (cleanedDevice[0] == 'G' && cleanedDevice[1] == 'D' &&
                cleanedDevice[2] == ' ')
            {
                char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN, "GD ",
                    "GD");
                free(cleanedDevice);
                cleanedDevice = tmp;
            }
        }
    }
    // Matrox Electronics Systems Ltd.
    else if (vendor[0] == 'M' && strncmp(vendor, "Matrox", 6) == 0)
    {
        cleanedVendor = strdup("Matrox");

        // If we have bracketed info, we discard the norm (usually just
        // containing the chip model name like 2064)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);
    }
    // S3 Graphics Ltd.
    else if (vendor[0] == 'S' && strncmp(vendor, "S3 ", 3) == 0)
    {
        cleanedVendor = strdup("S3 Graphics");

        // If we have bracketed info, we discard the norm (usually just
        // containing the core name)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);
    }
    else if (vendor[0] == 'T')
    {
        // Trident Microsystems
        if (strncmp(vendor, "Trident", 7) == 0)
            cleanedVendor = strdup("Trident");
        // Tseng Labs Inc
        else if (strncmp(vendor, "Tseng", 5) == 0)
            cleanedVendor = strdup("Tseng Labs");
    }
    // VMware
    else if (vendor[0] == 'V' && strncmp(vendor, "VMware", 6) == 0)
        cleanedVendor = strdup("VMware");
    // Anything else...
    else
    {
        // Apply generic deletions to vender name
        cleanedVendor = strdup(vendor);
        for (int i = 0; i < DELETIONS_LEN; i++)
        {
            const char *pattern = DELETIONS[i];
            char *tmp = findErase(cleanedVendor, GPU_NAME_LEN, pattern);
            free(cleanedVendor);
            cleanedVendor = tmp;
        }
    }

    // Apply generic deletions to device name
    for (int i = 0; i < DELETIONS_LEN; i++)
    {
        const char *pattern = DELETIONS[i];
        char *tmp = findErase(cleanedDevice, GPU_NAME_LEN, pattern);
        free(cleanedDevice);
        cleanedDevice = tmp;
    }

    // Combine and return final result
    snprintf(result, RESULT_SIZE, "%s %s", cleanedVendor, cleanedDevice);

    // Compact mode specific cleaning
    if (COMPACT && !isGPUFromCPU)
    {
        // Apply compact-specific GPU name shortenings
        int replaces = 0;
        for (int i = 0; i < COMPACT_GPU_REPLACES_LEN; i++)
        {
            if (COMPACT_GPU_REPLACES[i].standalone && replaces > 0)
                continue;
            else if (strstr(result, COMPACT_GPU_REPLACES[i].match))
            {
                char *tmp = findReplace(result, RESULT_SIZE,
                    COMPACT_GPU_REPLACES[i].match,
                    COMPACT_GPU_REPLACES[i].replacement);
                strncpy(result, tmp, RESULT_SIZE - 1);
                result[RESULT_SIZE - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }
    }

    free(cleanedVendor);
    free(cleanedDevice);
    free(cleanedDeviceNorm);
    free(cleanedDeviceBrac);

    return result;
}

#else

char *cleanGPUName(const char *vendor, const char *device,
    const int isGPUFromCPU)
{
    if (!vendor || !device)
        return strdup("");

    // Prepare result strings
    const int RESULT_SIZE = (GPU_NAME_LEN * 2) + 1;
    char *result = malloc(RESULT_SIZE);
    if (!result) return strdup("");

    snprintf(result, RESULT_SIZE, "%s %s", vendor, device);
    return result;
}

#endif

/**
 * @param count Number of GPUs actually detected (intended to be used by
 *              reference)
 * @return Pointer to up to MAX_GPUS of GPU_IDS structs containing detected
 *         GPUs
 */
GPU_IDS* getGPUs(int *count)
{
    if (!count)
        return NULL;

    DIR *dir = opendir("/sys/bus/pci/devices");
    if (!dir)
    {
        *count = 0;
        return NULL;
    }

    struct dirent *entry;
    GPU_IDS *gpus = malloc(MAX_GPUS * sizeof(GPU_IDS));
    if (!gpus) 
    {
        *count = 0;
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        char classPath[PATH_MAX];
        snprintf(classPath, sizeof(classPath), "%s/%s/class",
            "/sys/bus/pci/devices", entry->d_name);
        int class = readHexFile(classPath);
        class = (class >> 8) & 0xFFFF;

        // We only want class 0x30x...
        if ((class >> 8) == 0x03 && class != 0x0380)
        {
            char vendorPath[PATH_MAX], devicePath[PATH_MAX],
                revisionPath[PATH_MAX];
            snprintf(vendorPath, sizeof(vendorPath), "%s/%s/vendor",
                "/sys/bus/pci/devices", entry->d_name);
            snprintf(devicePath, sizeof(devicePath), "%s/%s/device",
                "/sys/bus/pci/devices", entry->d_name);
            snprintf(revisionPath, sizeof(revisionPath), "%s/%s/revision",
                "/sys/bus/pci/devices", entry->d_name);

            int vendor = readHexFile(vendorPath);
            int device = readHexFile(devicePath);
            int revision = readHexFile(revisionPath);

            if (EXCLUDED_PCI_DIDS_LEN > 0)
            {
                int excluded = 0;
                for (int i = 0; i < EXCLUDED_PCI_DIDS_LEN; i++)
                {
                    if (EXCLUDED_PCI_DIDS[i] == device)
                    {
                        excluded = 1;
                        break;
                    }
                }
                if (excluded)
                    continue;
            }

            gpus[*count].vendor = vendor;
            gpus[*count].device = device;
            gpus[*count].revision = revision;
            (*count)++;

            if (*count == MAX_GPUS)
                break;
        }
    }
    closedir(dir);

    return gpus;
}

/**
 * @param gpu GPU_IDS struct containing detected vendor and device IDs and
 *            revision number
 * @param os String containing the OS name (used for OS-specific checks)
 * @return String containing the GPU's assembled and cleaned full name;
 *         vendor nd device IDs as hex if interpreting failed
 */
char *interpretGPU(GPU_IDS *gpu, const char *os)
{
    char *gpuStr = malloc(GPU_NAME_LEN);
    if (!gpuStr)
        return strdup("unknown");
    gpuStr[0] = '\0';



#ifndef NO_STR_CLEANING

    // If Intel GPU, query our pre-defined iGPU list
    if (gpu->vendor == 0x8086)
    {
        const char *name = INTEL_IGPUS[gpu->device];
        if (name)
        {
            char *tmp = cleanGPUName("Intel", name, 0);
            strncpy(gpuStr, tmp, GPU_NAME_LEN - 1);
            gpuStr[GPU_NAME_LEN - 1] = '\0';
            free(tmp);
            return gpuStr;
        }
    }
    // If AMD GPU, query the AMD GPU IDs database
    else if (gpu->vendor == 0x1002)
    {
        // Possible paths to amdgpu.ids 
        char userAMDGPUIDs[PATH_MAX];
        snprintf(userAMDGPUIDs, PATH_MAX,
            "%s/.local/share/libdrm/amdgpu.ids", HOME);
        const char *amdGPUIDs[] = {
            "/usr/share/libdrm/amdgpu.ids",
            userAMDGPUIDs
        };

        for (int i = 0; i < 2; i++)
        {
            FILE *fStream = fopen(amdGPUIDs[i], "r");
            if (!fStream) continue;

            char line[256];
            while (fgets(line, sizeof(line), fStream))
            {
                if (line[0] == '#' || line[0] == '\n')
                    continue;

                int fileDID, fileRev;
                char name[256];
                // A line looks lile: 7480,	C1,	AMD Radeon RX 7700S
                if (sscanf(line, "%x,\t%x,\t%255[^\n]", &fileDID, &fileRev,
                    name) == 3)
                {
                    if (fileDID == gpu->device && fileRev == gpu->revision)
                    {
                        char *tmp = cleanGPUName("Advanced Micro", name, 0);
                        strncpy(gpuStr, tmp, GPU_NAME_LEN - 1);
                        gpuStr[GPU_NAME_LEN - 1] = '\0';
                        free(tmp);
                        fclose(fStream);
                        return gpuStr;
                    }
                }
            }
            fclose(fStream);
        }
    }

#endif



    // Check the PCI IDs database
    char *pciids;
    if (access("/usr/share/misc/pci.ids", F_OK) == 0)
        pciids = "/usr/share/misc/pci.ids";
    else if (access("/usr/share/hwdata/pci.ids", F_OK) == 0)
        pciids = "/usr/share/hwdata/pci.ids";
    else if (os && strstr(os, "NixOS") != NULL)
    {
        DIR *store = opendir("/nix/store");
        if (store)
        {
            static char nixPciIds[PATH_MAX];
            struct dirent *entry;
            while ((entry = readdir(store)) != NULL)
            {
                snprintf(nixPciIds, PATH_MAX,
                    "/nix/store/%s/share/hwdata/pci.ids", entry->d_name);
                if (access(nixPciIds, F_OK) == 0)
                {
                    pciids = nixPciIds;
                    break;
                }
            }
            closedir(store);
        }
    }
    else
    {
        snprintf(gpuStr, GPU_NAME_LEN, "%04x:%04x", gpu->vendor,
            gpu->device);
        return gpuStr;
    }

    FILE *fStream = fopen(pciids, "r");
    if (!fStream)
    {
        snprintf(gpuStr, GPU_NAME_LEN, "%04x:%04x", gpu->vendor,
            gpu->device);
        return gpuStr;
    }

    char *vendor = NULL;
    char vendorHex[5];
    sprintf(vendorHex, "%04x", gpu->vendor);
    char *device = NULL;
    char deviceHex[5];
    sprintf(deviceHex, "%04x", gpu->device);

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), fStream))
    {
        if (buffer[0] == '#' || buffer[0] == 'C' || buffer[0] == '\0')
            continue;
        
        if (strncmp(buffer, vendorHex, 4) == 0)
        {
            char *start = buffer + 6;
            int len = strlen(start);
            if (len > 0 && start[len - 1] == '\n')
                start[len - 1] = '\0';
            vendor = strdup(start);
        }

        if (vendor)
        {
            int tabs = 0;
            while (buffer[tabs] == '\t') tabs++;

            if (tabs > 0)
            {
                if (strncmp(buffer + tabs, deviceHex, 4) == 0)
                {
                    char *start = buffer + tabs + 6;
                    int len = strlen(start);
                    if (len > 0 && start[len - 1] == '\n')
                        start[len - 1] = '\0';
                    device = strdup(start);
                    break;
                }
            }
        }
    }
    fclose(fStream);

    if (!vendor || !device)
        snprintf(gpuStr, GPU_NAME_LEN, "%04x:%04x", gpu->vendor,
        gpu->device);
    else
        snprintf(gpuStr, GPU_NAME_LEN, "%s", 
            cleanGPUName(vendor, device, 0));

    if (vendor)
        free(vendor);
    if (device)
        free(device);

    return gpuStr;
}
