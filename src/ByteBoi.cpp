#include "ByteBoi.h"
#include <SPIFFS.h>
#include "SD_OTA.h"
#include <SD.h>
#include <SPI.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <PropertiesParser.h>
#include <iostream>
#include "GameDefaults.hpp"

const char* SPIFFSgameRoot = "/game/";
const char* SPIFFSdataRoot = "/data/";
using namespace std;
using namespace cppproperties;

ByteBoiImpl ByteBoi;
std::vector<Properties> ByteBoiImpl::games;

void ByteBoiImpl::begin(){
	if(psramFound()){
		Serial.printf("PSRAM init: %s, free: %d B\n", psramInit() ? "Yes" : "No", ESP.getFreePsram());
	}else{
		Serial.println("No PSRAM detected");
	}
	if(!SPIFFS.begin()){
		Serial.println("SPIFFS error");
		for(;;);
	}

	SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS);
	SPI.setFrequency(60000000);
	if(!SD.begin(SD_CS, SPI)){
		Serial.println("No SD card");
		for(;;);
	}
	Serial.println("SD ok");
	scanGames();

	display = new Display(160, 120, -1, 1);
	expander = new I2cExpander();

	display->begin();
	display->getBaseSprite()->clear(TFT_BLACK);
	display->commit();

	expander->begin(0x74, 23, 22);
	expander->pinMode(BL_PIN, OUTPUT);
	expander->pinWrite(BL_PIN, 0);

	input = new InputI2C(expander);
	input->preregisterButtons({ BTN_A, BTN_B, BTN_C, BTN_UP, BTN_DOWN, BTN_RIGHT, BTN_LEFT });

	//Piezo.begin(BUZZ_PIN);
}

File ByteBoiImpl::openResource(String path, const char* mode){
	if(!SPIFFS.exists(SPIFFSgameRoot) || !SPIFFS.exists(path)) return File();
	return SPIFFS.open(String(SPIFFSgameRoot) + path, mode);
}

File ByteBoiImpl::openData(String path, const char* mode){
	if(gameID.length() == 0) return File(); //undefined game ID

	if(!SPIFFS.exists(SPIFFSdataRoot)){
		SPIFFS.mkdir(SPIFFSdataRoot + gameID + "/");
	}
	return SPIFFS.open(SPIFFSdataRoot + gameID + "/" + path, mode);
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

void ByteBoiImpl::loadGame(size_t index){
	if(!inFirmware()) return;
	if(index >= games.size()) return;

	if(!SPIFFS.exists(SPIFFSgameRoot)){
		SPIFFS.mkdir(SPIFFSgameRoot);
	}
	File root = SPIFFS.open(SPIFFSgameRoot);
	File file = root.openNextFile();

	while(file){
		SPIFFS.remove(file.name());
		file = root.openNextFile();
	}
	file.close();
	root.close();

	char path[100];
	strncpy(path, "/", 100);
	strncat(path, getGameName(index), 100);
	strncat(path, "/", 100);
	strncat(path, getGameResources(index), 100);
	if(SD.exists(path)){
		root = SD.open(path);
		file = root.openNextFile();
		while(file){
			String fileName = file.name();
			fileName = fileName.substring(fileName.lastIndexOf('/') + 1);
			File destFile = SPIFFS.open(String(SPIFFSgameRoot) + fileName, FILE_WRITE);
			uint8_t buf[512];
			while(file.read(buf, 512)){
				destFile.write(buf, 512);
			}
			destFile.close();
			file = root.openNextFile();
		}
		file.close();
		root.close();
	}
	memset(path, 0, 100);
	strncpy(path, "/", 100);
	strncat(path, getGameName(index), 100);
	strncat(path, "/", 100);
	strncat(path, getGameBinary(index), 100);

	SD_OTA::updateFromSD(path);
}

void ByteBoiImpl::scanGames(){
	games.clear();
	File root = SD.open("/");
	File gameFolder = root.openNextFile();
	size_t counter = 0;
	while(gameFolder){
		if(gameFolder.isDirectory()){
			char path[100] = {0};
			strncat(path, gameFolder.name(), 100);
			strncat(path, "/game.properties", 100);

			if(SD.exists(path)){
				strncpy(path, "/sd", 100);
				strncat(path, gameFolder.name(), 100);
				strncat(path, "/game.properties", 100);

				Properties props = PropertiesParser::Read(path);
				std::string binaryPath = props.GetProperty("Binary");
				if(binaryPath == "") binaryPath = "firmware.bin";
				memset(path, 0, 100);
				strncat(path, gameFolder.name(), 100);
				strncat(path, "/", 100);
				strncat(path, binaryPath.c_str(), 100);
				if(SD.exists(path)){
					games.push_back(props);
				}
			}
		}
		gameFolder = root.openNextFile();
		counter++;
	}
	root.close();
	gameFolder.close();
}

fs::File ByteBoiImpl::getIcon(size_t index){
	if(index >= games.size()) return fs::File();

	char path[100] = {0};
	strncat(path, "/", 100);
	strncat(path, getGameName(index), 100);
	strncat(path, "/", 100);
	if(games[index].GetProperty("Icon", gameDefaults.icon).empty()){
		strncat(path, gameDefaults.icon, 100);
	}else{
		strncat(path, games[index].GetProperty("Icon", gameDefaults.icon).c_str(), 100);
	}
	return SD.open(path);
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

const std::vector<Properties> &ByteBoiImpl::getGameProperties(){
	return games;
}

const char* ByteBoiImpl::getGameName(size_t index){
	if(index >= games.size()) return nullptr;
	if(games[index].GetProperty("Name", gameDefaults.name).empty()){
		return gameDefaults.name;
	}
	return games[index].GetProperty("Name", gameDefaults.name).c_str();
}

const char* ByteBoiImpl::getGameBinary(size_t index){
	if(index >= games.size()) return nullptr;
	if(games[index].GetProperty("Binary", gameDefaults.binary).empty()){
		return gameDefaults.binary;
	}
	return games[index].GetProperty("Binary", gameDefaults.binary).c_str();
}

const char* ByteBoiImpl::getGameResources(size_t index){
	if(index >= games.size()) return nullptr;
	if(games[index].GetProperty("Resources", gameDefaults.resources).empty()){
		return gameDefaults.resources;
	}
	return games[index].GetProperty("Resources", gameDefaults.resources).c_str();
}

