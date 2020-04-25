/* acpi application yacpi
 * Copyright (C) 2005-2007 Nico Golde <nico@ngolde.de>
 * See COPYING file for license details
 */

/* define bitmask fields for command line arguments */

#define LOOP_F  (1<<0)
#define PTEXT_F (1<<1)
#define BATT_F  (1<<2)
#define AC_F    (1<<3)
#define FAN_F   (1<<4)
#define THERM_F (1<<5)
#define CPU_F   (1<<6)
#define GOV_F   (1<<7)
#define NON_F   (1<<8)
#define ALL_F   (BATT_F | AC_F | FAN_F | THERM_F | CPU_F | GOV_F)
