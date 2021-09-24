#include "ByteBoi.h"
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <PropertiesParser.h>
#include <iostream>
#include <utility>
#include "ByteBoiLED.h"
#include "Menu/Menu.h"
#include "Settings.h"

const char* ByteBoiImpl::SPIFFSgameRoot = "/game/";
const char* ByteBoiImpl::SPIFFSdataRoot = "/data/";
using namespace std;

ByteBoiImpl ByteBoi;
BatteryService Battery;

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
	if(!SD.begin(SD_CS, SPI)){
		Serial.println("No SD card");
		for(;;);
	}
	Serial.println("SD ok");

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

	Settings.begin();

	Context::setDeleteOnPop(true);

	bindMenu();

	Piezo.begin(SPEAKER_PIN);
	Piezo.setMute(Settings.get().mute);
	Battery.begin();
}

File ByteBoiImpl::openResource(const String& path, const char* mode){
	if(!SPIFFS.exists(SPIFFSgameRoot) || !SPIFFS.exists(path)) return File();
	return SPIFFS.open(String(SPIFFSgameRoot) + path, mode);
}

File ByteBoiImpl::openData(const String& path, const char* mode){
	if(gameID.length() == 0) return File(); //undefined game ID

	if(!SPIFFS.exists(SPIFFSdataRoot)){
		SPIFFS.mkdir(SPIFFSdataRoot + gameID + "/");
	}
	return SPIFFS.open(SPIFFSdataRoot + gameID + "/" + path, mode);
}

bool ByteBoiImpl::inFirmware(){
	return (strcmp(esp_ota_get_boot_partition()->label, "app0") == 0); //already in launcher partition
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

