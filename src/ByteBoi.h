#ifndef BYTEBOI_H
#define BYTEBOI_H

#define BTN_UP 0
#define BTN_DOWN 3
#define BTN_LEFT 1
#define BTN_RIGHT 2
#define BTN_A 6
#define BTN_B 5
#define BTN_C 4

#define BUZZ_PIN 12
#define BL_PIN 12

#define LED_G 13
#define LED_B 15
#define LED_R 14
#define TT1 &TomThumb

#include <Arduino.h>
#include <CircuitOS.h>
#include <Display/Display.h>
#include <Display/Sprite.h>
#include <Input/I2cExpander.h>
#include <Input/InputI2C.h>
#include <Audio/Piezo.h>

class ByteBoiImpl {
public:

	void setDataRoot(String dataRoot);

	void open(String path, const char* mode);

	/**
	 * Initializes display, backlight, Piezo, I2C expander, I2C input, and pre-registers all buttons.
	 */
	void begin();

	Display* getDisplay();
	I2cExpander* getExpander();
	InputI2C* getInput();

private:
	String dataRoot;
	Display* display;
	I2cExpander* expander;
	InputI2C* input;

};

extern ByteBoiImpl ByteBoi;

#endif //BYTEBOI_H
