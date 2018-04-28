#include <stdbool.h>
#include <stdint.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/catalog/arm/cortexm4/tiva/ce/sysctl.h>
#include "drivers/pinout.h"
#include "ui/main.h"

/*
 *  ======== main ========
 */
Int main()
{
    /*
     * use ROV->SysMin to view the characters in the circular buffer
     */
    System_printf("enter main()\n");
    uint32_t ui32SysClock;

    ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                           SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                                           SYSCTL_CFG_VCO_480), 120000000);
    // Configure the device pins
    PinoutSet();

    ui_setup(ui32SysClock);

    BIOS_start();    /* does not return */
    return(0);
}
