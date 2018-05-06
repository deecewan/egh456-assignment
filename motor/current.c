#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

// May not have to use these two constants at all:
#define VIOUT_MAX VCC + 0.5 // Maximum output signal (page 3 of current sensor datasheet)
#define VIOUT_MIN VCC * 0.1 // Zero current output signal (page 23 of current sensor datasheet)

#define VCC 24 // According to assignment document
#define FIRST_STEP 0
#define SEQUENCE_NUMBER 0 // Could be any value from 0 to 3 since only one sample is needed at a given request
#define ADC_PRIORITY 0
#define RESOLUTION 4095 // max digital value for 12 bit sample
#define REF_VOLTAGE_PLUS 3.3 // Reference voltage used for ADC process, given in page 2149 of TM4C129XNCZAD Microcontroller Data Sheet

// Function prototypes
void StartADCSampling();
double GetCurrentValue();

/*
 * Starts the ADC sampling hardware for the current line.
 */
void StartADCSampling() {
    // Initialise ADC hardware (Page 11 of DK-TM4C129X User's Guide has relevant board pin information)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    //
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This example is arbitrarily using sequence 3.
    //
    ADCSequenceConfigure(ADC0_BASE, SEQUENCE_NUMBER, ADC_TRIGGER_PROCESSOR, ADC_PRIORITY);
    // ADCSequenceConfigure(ADC0_BASE, SEQUENCE_NUMBER, ADC_TRIGGER_ALWAYS, ADC_PRIORITY);

    //
    // Configure step 0 on sequence 0.  Sample channel 0 (ADC_CTL_CH0) in
    // differential mode (ADC_CTL_D) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.   For more
    // information on the ADC sequences and steps, refer to the datasheet.
    //
    ADCSequenceStepConfigure(ADC0_BASE, SEQUENCE_NUMBER, FIRST_STEP, ADC_CTL_IE | ADC_CTL_CH0 | ADC_CTL_END);

    //
    // Since sample sequence 0 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, SEQUENCE_NUMBER);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, SEQUENCE_NUMBER);
}

/*
 * Gets the most recent VIout sample stored by ADC sampler and calculates current value from that.
 */
double GetCurrentValue() {
    //
    // This array is used for storing the data read from the ADC FIFO. It
    // must be as large as the FIFO for the sequencer in use. This example
    // uses sequence 3 which has a FIFO depth of 1. If another sequence
    // was used with a deeper FIFO, then the array size must be changed.
    //
    uint32_t pui32ADC0Value[1], twelve_bitmask = 0xfff;
    double current_value = 1.25, VIOUT = 0; // NEED TO CHANGE THIS TO ACCURATELY CHANGE DIGITISED VALUE TO CURRENT VALUE

    // Trigger the ADC conversion.
    ADCProcessorTrigger(ADC0_BASE, SEQUENCE_NUMBER);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC0_BASE, SEQUENCE_NUMBER, false));

    //Clear ADC Interrupt
    ADCIntClear(ADC0_BASE, SEQUENCE_NUMBER);

    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, SEQUENCE_NUMBER, pui32ADC0Value);

    // Convert digital value to current reading (VREF- is 0, so it can be ignored)
    VIOUT = ((pui32ADC0Value[0] & twelve_bitmask) * REF_VOLTAGE_PLUS) / RESOLUTION;
    current_value = VIOUT / VCC;
    return current_value;
}
