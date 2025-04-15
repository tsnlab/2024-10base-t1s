#pragma once

#include <linux/ioctl.h>

/* Command definition */
#define LAN865X_MAGIC 'L'                                            /* Driver's unique magic number */
#define LAN865X_READ_REG _IOR(LAN865X_MAGIC, 1, struct lan865x_reg)  /* Read command */
#define LAN865X_WRITE_REG _IOW(LAN865X_MAGIC, 2, struct lan865x_reg) /* Write command */

/* register access structure */
struct lan865x_reg {
    u32 addr;  /* register address */
    u32 value; /* register value */
};
