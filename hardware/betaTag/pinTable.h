/*
 * pinTable.h
 *
 *  Created on: Nov 10, 2016
 *      Author: annik
 */

#ifndef PINTABLE_H_
#define PINTABLE_H_

/*leds*/
#define NODE_ACTIVITY_LED Board_LED0 			// for beta
#define CONCENTRATOR_ACTIVITY_LED Board_LED1 	// for alpha

/* Global memory storage for a PIN_Config table */
extern PIN_State ledPinState;
extern PIN_Handle ledPinHandle;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
static PIN_Config ledPinTable[] = {
		Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, // rf
		Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, //
		Board_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
		PIN_TERMINATE
};

/************************/

extern PIN_Handle buttonPinHandle;
extern PIN_State buttonPinState;
/*
 * Application button pin configuration table:
 *   - Buttons interrupts are configured to trigger on falling edge.
 */
static PIN_Config buttonPinTable[] = {
		Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
		PIN_TERMINATE
};


#endif /* PINTABLE_H_ */
