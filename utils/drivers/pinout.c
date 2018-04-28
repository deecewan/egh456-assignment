//*****************************************************************************
//
// pinout.c - Function to configure the device pins on the DK-TM4C129X.
//
// Copyright (c) 2013 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.0.1.11577 of the DK-TM4C129X Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "drivers/pinout.h"

//*****************************************************************************
//
//! \addtogroup pinout_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Configures the device pins for the standard usages on the DK-TM4C129X.
//!
//! This function enables the GPIO modules and configures the device pins for
//! the default, standard usages on the DK-TM4C129X.  Applications that require
//! alternate configurations of the device pins can either not call this
//! function and take full responsibility for configuring all the device pins,
//! or can reconfigure the required device pins after calling this function.
//!
//! \return None.
//
//*****************************************************************************
void
PinoutSet(void)
{
    //
    // Enable all the GPIO peripherals.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOR);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOS);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOT);

    //
    // PA0-1 are used for UART0.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // PA2-5 are used for SSI0 to the second booster pack.
    //
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);
    GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);

    //
    // PB0-1/PD6-7/PL6-7 are used for USB.
    //
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0xff;
    GPIOPinConfigure(GPIO_PD6_USB0EPEN);
    GPIOPinConfigure(GPIO_PD7_USB0PFLT);
    GPIOPinTypeUSBAnalog(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypeUSBDigital(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    GPIOPinTypeUSBAnalog(GPIO_PORTL_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    //
    // PB2/PD4 are used for the speaker output.
    //
    GPIOPinConfigure(GPIO_PB2_T5CCP0);
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_4, 0);

    //
    // PB6-7 are used for I2C to the TMP100 and the EM connector.
    //
    GPIOPinConfigure(GPIO_PB6_I2C6SCL);
    GPIOPinConfigure(GPIO_PB7_I2C6SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_7);

    //
    // PE5/PN3/PP1 are used for the push buttons.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_5);
    GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_1);

    //
    // PE7/PP7/PT2-3 are used for the touch screen.
    //
    HWREG(GPIO_PORTE_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTE_BASE + GPIO_O_CR) = 0xff;
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_7);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_7);
    GPIOPinTypeGPIOInput(GPIO_PORTT_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_7, 0);
    GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_7, 0);
    GPIOPinTypeGPIOOutput(GPIO_PORTT_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTT_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0);

    //
    // PF0/PF4-5/PH4/PQ0-2 are used for the SPI flash (on-board and SD card).
    // PH4 selects the SD card and PQ1 selects the on-board SPI flash.
    //
    GPIOPinConfigure(GPIO_PF0_SSI3XDAT1);
    GPIOPinConfigure(GPIO_PF4_SSI3XDAT2);
    GPIOPinConfigure(GPIO_PF5_SSI3XDAT3);
    GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
    GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0);
    GPIOPinTypeSSI(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_4, GPIO_PIN_4);
    GPIOPinTypeSSI(GPIO_PORTQ_BASE, GPIO_PIN_0 | GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_1, GPIO_PIN_1);

    //
    // PF1/PK4/PK6 are used for Ethernet LEDs.
    //
    GPIOPinConfigure(GPIO_PF1_EN0LED2);
    GPIOPinConfigure(GPIO_PK4_EN0LED0);
    GPIOPinConfigure(GPIO_PK6_EN0LED1);
    GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinTypeEthernetLED(GPIO_PORTK_BASE, GPIO_PIN_4);
    GPIOPinTypeEthernetLED(GPIO_PORTK_BASE, GPIO_PIN_6);

    //
    // PF6-7/PJ6/PS4-5/PR0-7 are used for the LCD.
    //
    GPIOPinConfigure(GPIO_PF7_LCDDATA02);
    GPIOPinConfigure(GPIO_PJ6_LCDAC);
    GPIOPinConfigure(GPIO_PR0_LCDCP);
    GPIOPinConfigure(GPIO_PR1_LCDFP);
    GPIOPinConfigure(GPIO_PR2_LCDLP);
    GPIOPinConfigure(GPIO_PR3_LCDDATA03);
    GPIOPinConfigure(GPIO_PR4_LCDDATA00);
    GPIOPinConfigure(GPIO_PR5_LCDDATA01);
    GPIOPinConfigure(GPIO_PR6_LCDDATA04);
    GPIOPinConfigure(GPIO_PR7_LCDDATA05);
    GPIOPinConfigure(GPIO_PS4_LCDDATA06);
    GPIOPinConfigure(GPIO_PS5_LCDDATA07);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_6);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_6, GPIO_PIN_6);
    GPIOPinTypeLCD(GPIO_PORTF_BASE, GPIO_PIN_7);
    GPIOPinTypeLCD(GPIO_PORTJ_BASE, GPIO_PIN_6);
    GPIOPinTypeLCD(GPIO_PORTR_BASE,
                       (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                        GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7));
    GPIOPinTypeLCD(GPIO_PORTS_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // PQ7 is used for the user LED.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_7 | GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_7 | GPIO_PIN_4 , 0);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
