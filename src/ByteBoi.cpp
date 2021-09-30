#include "ByteBoi.h"
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <Loop/LoopManager.h>
#include "ByteBoiLED.h"
#include "Menu/Menu.h"
#include "Settings.h"
#include "Battery/BatteryPopupService.h"
#include "SleepService.h"
#include <Loop/LoopManager.h>
#include <esp_wifi.h>

const char* ByteBoiImpl::SPIFFSgameRoot = "/game";
const char* ByteBoiImpl::SPIFFSdataRoot = "/data";
using namespace std;
using namespace MiniMenu;

ByteBoiImpl ByteBoi;
BatteryService Battery;
BatteryPopupService BatteryPopup;
Menu* ByteBoiImpl::popupMenu = nullptr;

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

	Piezo.begin(SPEAKER_PIN);
	Piezo.setMute(Settings.get().volume);

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
	input->addListener(this);
}

void ByteBoiImpl::unbindMenu(){
	menuBind = false;
	input->removeListener(this);
}

void ByteBoiImpl::buttonPressed(uint i){
	if(!menuBind){
		input->removeListener(this);
		return;
	}

	if(i == BTN_C || (popupMenu != nullptr && i == BTN_B)){
		if(ContextTransition::isRunning() || ModalTransition::isRunning()) return;

		if(popupMenu == nullptr){
			openMenu();
		}else if(popupMenu != nullptr){
			Menu::popIntoPrevious();
			popupMenu = nullptr;
		}
	}
}

void ByteBoiImpl::openMenu(){
	if(ContextTransition::isRunning() || ModalTransition::isRunning()) return;
	if(Modal::getCurrentModal() != nullptr){
		ModalTransition::setDeleteOnPop(false);
		auto transition = static_cast<ModalTransition *>((void *)Modal::getCurrentModal()->pop());
		transition->setDoneCallback([](Context *currentContext, Modal *prevModal){
			ByteBoiImpl::popupMenu = new Menu(currentContext);
			ByteBoiImpl::popupMenu->push(currentContext);
			ByteBoiImpl::popupMenu->returned(prevModal);
			ModalTransition::setDeleteOnPop(true);
		});
	}else{
		ByteBoiImpl::popupMenu = new Menu(Context::getCurrentContext());
		ByteBoiImpl::popupMenu->push(Context::getCurrentContext());
	}
}

void ByteBoiImpl::shutdown(){
	display->getTft()->sleep();
	expander->pinMode(BL_PIN, 1);
	LED.setRGB(OFF);
	esp_wifi_stop();
	btStop();
	Piezo.noTone();
	esp_deep_sleep_start();
}

