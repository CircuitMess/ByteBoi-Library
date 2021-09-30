#include "ByteBoi.h"
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <PropertiesParser.h>
#include <iostream>
#include <utility>
#include <Loop/LoopManager.h>
#include "ByteBoiLED.h"
#include "Menu/Menu.h"
#include "Settings.h"
#include "Battery/BatteryPopupService.h"
#include "SleepService.h"
#include <Loop/LoopManager.h>
#include <WiFi.h>
#include <FS/CompressedFile.h>

const char* ByteBoiImpl::SPIFFSgameRoot = "/game";
const char* ByteBoiImpl::SPIFFSdataRoot = "/data";
using namespace std;

ByteBoiImpl ByteBoi;
BatteryService Battery;
BatteryPopupService BatteryPopup;

void ByteBoiImpl::begin(){

	if(!inFirmware()){
		esp_ota_set_boot_partition(esp_ota_get_next_update_partition(esp_ota_get_running_partition()));
	}

	if(psramFound()){
		Serial.printf("PSRAM init: %s, free: %d B\n", psramInit() ? "Yes" : "No", ESP.getFreePsram());
	}else{
		Serial.println("No PSRAM detected");
	}
	if(!SPIFFS.begin()){
		Serial.println("SPIFFS error");
		SPIFFS.begin(true);
		ESP.restart();
//		for(;;);
	}

	SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS);
	SPI.setFrequency(60000000);

	display = new Display(160, 120, -1, 1);
	expander = new I2cExpander();

	display->begin();
	display->getBaseSprite()->clear(TFT_BLACK);
	display->commit();

	expander->begin(0x74, 23, 22);
	expander->pinMode(BL_PIN, OUTPUT);
	expander->pinWrite(BL_PIN, 0);
	LED.begin();
	LED.setRGB(OFF);

	input = new InputI2C(expander);
	input->preregisterButtons({ BTN_A, BTN_B, BTN_C, BTN_UP, BTN_DOWN, BTN_RIGHT, BTN_LEFT });
	LoopManager::addListener(Input::getInstance());

	Settings.begin();

	Context::setDeleteOnPop(true);

	bindMenu();

	Piezo.begin(SPEAKER_PIN);
	Piezo.setMute(Settings.get().mute);

	Battery.begin();
	LoopManager::addListener(&Sleep);
	input->addListener(&Sleep);

}

File ByteBoiImpl::openResource(const String& path, const char* mode){
	if(strcmp(esp_ota_get_running_partition()->label, "game") == 0){
		String result = String(SPIFFSgameRoot + path);
		if(!SPIFFS.exists(result)) return File();
		return SPIFFS.open(result, mode);
	}else{
		return SPIFFS.open(path, mode);
	}

}

File ByteBoiImpl::openData(const String& path, const char* mode){
	if(strcmp(esp_ota_get_running_partition()->label, "game") == 0){
		if(gameID.length() == 0) return File(); //undefined game ID
		return SPIFFS.open(String(SPIFFSdataRoot) + "/" + gameID + "/" + path, mode);
	}else{
		return SPIFFS.open(path, mode);
	}

}

bool ByteBoiImpl::inFirmware(){
	return (strcmp(esp_ota_get_running_partition()->label, "launcher") == 0); //already in launcher partition
}

void ByteBoiImpl::backToLauncher(){
	if(inFirmware()) return;

	const esp_partition_t *partition = esp_ota_get_running_partition();
	const esp_partition_t *partition2 = esp_ota_get_next_update_partition(partition);
	esp_ota_set_boot_partition(partition2);
	ESP.restart();
}

Display* ByteBoiImpl::getDisplay(){
	return display;
}

I2cExpander* ByteBoiImpl::getExpander(){
	return expander;
}

InputI2C* ByteBoiImpl::getInput(){
	return input;
}

void ByteBoiImpl::setGameID(String ID){
	gameID = std::move(ID);
}

void ByteBoiImpl::bindMenu(){
	menuBind = true;
}

void ByteBoiImpl::unbindMenu(){
	menuBind = false;
}

void ByteBoiImpl::buttonPressed(uint i){
	if(!menuBind) return;
	if(i == BTN_C){
		Menu* menu = new Menu(Context::getCurrentContext());
		menu->push(Context::getCurrentContext());
	}
}

void ByteBoiImpl::shutdown(){
	display->getTft()->sleep();
	expander->pinMode(BL_PIN, 1);
	LED.setRGB(OFF);
	WiFi.mode(WIFI_OFF);
	btStop();
	Piezo.noTone();
	esp_deep_sleep_start();
}

void ByteBoiImpl::splash(void(* callback)()){
	Color* logoBuffer = nullptr;
	fs::File logoFile = SPIFFS.open("/launcher/ByteBoiLogo.raw");
	if(!logoFile){
		Serial.println("Error opening logo in splash");
		return;
	}
	logoFile.read(reinterpret_cast<uint8_t*>(logoBuffer), 93 * 26 * 2);
	logoFile.close();
	display->getBaseSprite()->clear(C_HEX(0x0041ff));
	display->getBaseSprite()->drawIcon(logoBuffer, (display->getWidth() / 2) - 46, (display->getHeight() / 2) - 13, 93, 26);
	display->commit();
	if(callback == nullptr){
		delay(1000);
	}else{
		LoopManager::addListener(this);
	}


}

void ByteBoiImpl::loop(uint micros){
	if(millis() > 1000){
		LoopManager::removeListener(this);
	}
}


