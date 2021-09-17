#ifndef BYTEBOI_H
#define BYTEBOI_H

#include <Arduino.h>
#include <CircuitOS.h>
#include <Display/Display.h>
#include <Display/Sprite.h>
#include <Input/I2cExpander.h>
#include <Input/InputI2C.h>
#include <Audio/Piezo.h>
#include <vector>
#include <FS.h>
#include "Pins.hpp"

class ByteBoiImpl {
public:

	/**
	 * Initializes display, backlight, Piezo, I2C expander, I2C input, and pre-registers all buttons.
	 */
	void begin();
	Display* getDisplay();
	I2cExpander* getExpander();
	InputI2C* getInput();
	static bool inFirmware();

	//functions to be used by games
	File openResource(const String& path, const char* mode);
	File openData(const String& path, const char* mode);
	void setGameID(String ID);
	void backToLauncher();
	void bindMenu();
	void unbindMenu();

	static const char* SPIFFSgameRoot;
	static const char* SPIFFSdataRoot;

private:
	Display* display;
	I2cExpander* expander;
	InputI2C* input;
	String gameID = "";
};

extern ByteBoiImpl ByteBoi;

#endif //BYTEBOI_H
