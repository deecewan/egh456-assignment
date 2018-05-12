#include <stdbool.h>
#include <stdint.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Seconds.h>
#include <ti/catalog/arm/cortexm4/tiva/ce/sysctl.h>
#include <inc/hw_memmap.h>
#include "drivers/pinout.h"
#include "motor/current.h"
#include "motor/speed.h"
#include "motor/temperature.h"
#include "motor/measurement.h"
#include "ui/main.h"

#define SECONDS_SINCE_EPOCH (1526136342 + (10 * 60 * 60)) // add 10 hours to account for the TZ difference

int initialise_hardware() {
    // call all hardware setup functions
    // if any return -1, we are in a faulty state
    StartADCSampling();
    ConnectWithTemperatureSensor();
    ConnectWithMotor();
    MeasurementInit();
    return ConnectWithHallSensors();
}

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

    Seconds_set(SECONDS_SINCE_EPOCH);

    // enable the LED pins to display motor status
    // red pin
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_5);
    // green pin
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_7);

    ui_setup(ui32SysClock, initialise_hardware());

    BIOS_start();    /* does not return */
    return(0);
}
