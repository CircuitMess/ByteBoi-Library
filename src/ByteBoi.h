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

#define SPI_SCK 26
#define SPI_MISO 5
#define SPI_MOSI 32
#define SPI_SS -1
#define SD_CS 2

#include <Arduino.h>
#include <CircuitOS.h>
#include <Display/Display.h>
#include <Display/Sprite.h>
#include <Input/I2cExpander.h>
#include <Input/InputI2C.h>
#include <Audio/Piezo.h>
#include <vector>
#include <FS.h>

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

	void loadGame(const char* game);
	std::vector<std::string>& scanGames();
	fs::File getIcon(const char* game);
	bool inFirmware();
	static const std::vector <std::string> &getGameNames();
	void backToLauncher();

private:
	String dataRoot;
	Display* display;
	I2cExpander* expander;
	InputI2C* input;
	static std::vector<std::string> gameNames;
};

extern ByteBoiImpl ByteBoi;

#endif //BYTEBOI_H
