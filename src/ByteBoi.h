#ifndef NIBBLE_NIBBLE_H
#define NIBBLE_NIBBLE_H

#include "Pins.hpp"

#include <CircuitOS.h>
#include <Display/Display.h>
#include <Display/Sprite.h>
#include <Input/I2cExpander.h>
#include <Input/InputI2C.h>
#include <Audio/Piezo.h>

class ByteBoiImpl {
public:

	/**
	 * Initializes display, backlight, Piezo, I2C expander, I2C input, and pre-registers all buttons.
	 */
	void begin();

	Display* getDisplay();
	I2cExpander* getExpander();
	InputI2C* getInput();

private:
	Display* display;
	I2cExpander* expander;
	InputI2C* input;

};

extern ByteBoiImpl ByteBoi;

#endif //NIBBLE_NIBBLE_H
