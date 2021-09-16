#include <Arduino.h>
#include "src/ByteBoi.h"
#include <Loop/LoopManager.h>
#include <SD.h>
#include <Display/Color.h>

void setup(){
	Serial.begin(115200);
	Serial.print("in firmware: ");
	Serial.println(ByteBoi.inFirmware());

	ByteBoi.begin();
	std::vector<std::string> gameNames = ByteBoi.scanGames();

	Color* appIconBuffer = static_cast<Color*>(ps_malloc(64 * 64 * 2));
	if(appIconBuffer == nullptr){
		Serial.println("MainMenuApp picture unpack error");
		return;
	}
	File file = ByteBoi.getIcon(gameNames[0].c_str());
	file.read(reinterpret_cast<uint8_t*>(appIconBuffer), 64 * 64 * 2);
	file.close();
	ByteBoi.getDisplay()->getBaseSprite()->drawIcon(appIconBuffer, 20, 20, 64, 64, 1, TFT_BLACK);
	ByteBoi.getDisplay()->commit();

	delay(2000);

	ByteBoi.loadGame(gameNames[0].c_str());


}

void loop(){
	LoopManager::loop();
}