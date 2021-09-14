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
#include <Properties.h>
#include "Pins.hpp"

using namespace cppproperties;

class ByteBoiImpl {
public:


	/**
	 * Initializes display, backlight, Piezo, I2C expander, I2C input, and pre-registers all buttons.
	 */
	void begin();
	Display* getDisplay();
	I2cExpander* getExpander();
	InputI2C* getInput();

	//functions used by the launcher/firmware
	void loadGame(size_t index);
	void scanGames();
	bool inFirmware();
	void backToLauncher();
	static const std::vector<Properties> &getGameProperties();
	const char* getGameName(size_t index);
	fs::File getIcon(size_t index);
	const char* getGameBinary(size_t index);
	const char* getGameResources(size_t index);

	//functions to be used by games
	File openResource(String path, const char* mode);
	File openData(String path, const char* mode);
	void setGameID(String ID);


private:
	Display* display;
	I2cExpander* expander;
	InputI2C* input;
	static std::vector<Properties> games;
	String gameID = "";
};

extern ByteBoiImpl ByteBoi;

#endif //BYTEBOI_H
