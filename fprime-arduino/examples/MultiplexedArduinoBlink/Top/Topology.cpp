#include <Fw/Types/Assert.hpp>
#include <Os/Task.hpp>
#include <Os/Log.hpp>
#include <Os/File.hpp>
#include <Os/Baremetal/TaskRunner/TaskRunner.hpp>
#include <Fw/Types/MallocAllocator.hpp>
#include <ATmega/examples/MultiplexedArduinoBlink/Top/ArduinoSchedContexts.hpp>
#include <Drv/ATmegaGpioDriver/ATmegaGpioDriverComponentImpl.hpp>
#ifdef ARDUINO
    #include <avr/io.h>
#else
    volatile uint8_t DDRB = 2;
    volatile uint8_t PORTB = 2;
    #define PB7 7
#endif
#include "Components.hpp"

// Setup the rate group driver used to drive all the ActiveRateGroups connected to it.
// For each active rate group, there is a rate divisor that represents how often it is run.
static NATIVE_INT_TYPE rate_divisors[] = {1, 10};
Svc::RateGroupDriverImpl rateGroupDriverComp("RGDRV", rate_divisors, FW_NUM_ARRAY_ELEMENTS(rate_divisors));

// Context array variables are passed to rate group members if needed to distinguish one call from another
// These context must match the rate group members connected in ArduinoTopologyAi.xml
static NATIVE_UINT_TYPE rg10HzContext[] = {Arduino::CONTEXT_ATMEGA_DEMO_1Hz, 0, 0, 0};
Svc::ActiveRateGroupImpl rateGroup10HzComp("RG10Hz",rg10HzContext,FW_NUM_ARRAY_ELEMENTS(rg10HzContext));
// static NATIVE_UINT_TYPE rg1HzContext[] = {0, 0, Arduino::CONTEXT_ATMEGA_DEMO_1Hz, 0};
// Svc::ActiveRateGroupImpl rateGroup1HzComp("RG1Hz",rg1HzContext,FW_NUM_ARRAY_ELEMENTS(rg1HzContext));

// Standard system components

// Arduino specific components
Arduino::LedBlinkerComponentImpl ledBlinker("Blinker");
Arduino::RateMultiplexerComponentImpl rateMultiplexer("RateMultiplexer");
Arduino::HardwareRateDriver hardwareRateDriver("RateDr", 100);
Drv::ATmegaGpioDriverComponentImpl ledGpio("ledGpio");

// Baremetal setup for the task runner
Os::TaskRunner taskRunner;
/**
 * Construct App:
 *
 * Constructs the App. It initialize components, call for command registration and
 * starts tasks. This is the initialization of the application, so new tasks and
 * memory can be acquired here, but should not be created at a later point.
 */
void constructApp() {
    // Initialize rate group driver
    rateGroupDriverComp.init();

    // Initialize the rate groups
    rateGroup10HzComp.init(10, 0);
    // rateGroup1HzComp.init(10, 1);

    // Initialize the core data handling components
    ledBlinker.init(0);
    rateMultiplexer.init(0);


    ledGpio.init(0);

#ifndef ARDUINO
    printf("Constructing Architecture.\n");
#endif
    // Callback to initialize architecture, connect ports, etc.
    constructArduinoArchitecture(); // HANGS HERE!!!!!

    // configure things
    ledGpio.setup(DDRB, PORTB, PB5, Drv::ATmegaGpioDriverComponentImpl::GPIO_OUT);

    hardwareRateDriver.start();

    // Start all active components' tasks thus finishing the setup portion of this code
    rateGroup10HzComp.start(0, 120, 10 * 1024);
    // rateGroup1HzComp.start(0, 119, 10 * 1024);

    // Start the task for the rate group
#ifndef ARDUINO
    printf("Starting TaskRunner.\n");
#endif
    while(1){taskRunner.run();}
}
/**
 * Exit Tasks:
 *
 * Exits the tasks in preparation for stopping the software. This is likely
 * not needed for Arduino projects, as they run forever, however; it is here
 * for completeness.
 */
void exitTasks(void) {
    // rateGroup1HzComp.exit();
    rateGroup10HzComp.exit();
}