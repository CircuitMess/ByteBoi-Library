#include "ByteBoi.h"
#include <SPIFFS.h>
#include "SD_OTA.h"
#include <SD.h>
#include <SPI.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>

ByteBoiImpl ByteBoi;
std::vector<std::string> ByteBoiImpl::gameNames;

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
		//for(;;);
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

	input = new InputI2C(expander);
	input->preregisterButtons({ BTN_A, BTN_B, BTN_C, BTN_UP, BTN_DOWN, BTN_RIGHT, BTN_LEFT });

	//Piezo.begin(BUZZ_PIN);
}

void ByteBoiImpl::setDataRoot(String dataRoot){
	ByteBoiImpl::dataRoot = dataRoot;
}

void ByteBoiImpl::open(String path, const char* mode){
	SPIFFS.open(String(dataRoot + path), mode);
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

void ByteBoiImpl::loadGame(const char* game){
	if(!inFirmware()) return;

	File root = SD.open(game);
	File file = root.openNextFile();

	while(file){
		if(strstr(file.name(), ".bin") == nullptr || strstr(file.name(), ".BIN") == nullptr ||
		   strstr(file.name(), ".icon") == nullptr || strstr(file.name(), ".ICON") == nullptr){

			File destFile = SPIFFS.open(file.name(), FILE_WRITE);
			uint8_t buf[512];
			while(file.read(buf, 512)){
				destFile.write(buf, 512);
			}
			destFile.close();
		}
		file = root.openNextFile();
	}

	char path[100];
	strncpy(path, "/", 100);
	strncat(path, game, 100);
	strncat(path, "/", 100);
	strncat(path, game, 100);
	strncat(path, ".bin", 100);
	SD_OTA::updateFromSD(path);

}

std::vector<std::string> &ByteBoiImpl::scanGames(){
	gameNames.clear();
	File root = SD.open("/");
	File gameFolder = root.openNextFile();
	while(gameFolder){
		if(gameFolder.isDirectory()){
			char path[100];
			strncpy(path, gameFolder.name(), 100);
			strncat(path, gameFolder.name(), 100);
			strncat(path, ".bin", 100);

			if(SD.exists(path)){

				gameNames.emplace_back(gameFolder.name() + 1);
			}
		}

		gameFolder = root.openNextFile();
	}
	return gameNames;
}

fs::File ByteBoiImpl::getIcon(const char* game){
	char path[100];
	strncpy(path, "/", 100);
	strncat(path, game, 100);
	strncat(path, "/", 100);
	strncat(path, game, 100);
	strncat(path, ".icon", 100);
	return SD.open(path);
}

bool ByteBoiImpl::inFirmware(){
	return (esp_partition_find_first(ESP_PARTITION_TYPE_APP , ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL) != NULL);
}

const std::vector <std::string> &ByteBoiImpl::getGameNames(){
	return gameNames;
}

void ByteBoiImpl::backToLauncher(){
	if(strcmp(esp_ota_get_boot_partition()->label, "app0") == 0) return; //already in launcher partition

	const esp_partition_t *partition = esp_ota_get_running_partition();
	const esp_partition_t *partition2 = esp_ota_get_next_update_partition(partition);
	esp_ota_set_boot_partition(partition2);
	ESP.restart();
}

