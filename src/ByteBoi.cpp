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
	if(gameID.length() == 0) return File(); //undefined game ID
	return SPIFFS.open(String(SPIFFSdataRoot) + "/" + gameID + path, mode);
}

bool ByteBoiImpl::inFirmware(){
	return (strcmp(esp_ota_get_running_partition()->label, "launcher") == 0); //already in launcher partition
}

bool ByteBoiImpl::isStandalone(){
	return (strcmp(esp_ota_get_running_partition()->label, "game") != 0);
}

void ByteBoiImpl::backToLauncher(){
	if(inFirmware() || isStandalone()) return;

	fadeout();
	expander->pinWrite(BL_PIN, HIGH);

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

void ByteBoiImpl::fadeout(){
	Sprite* canvas = display->getBaseSprite();

	Color* logoBuffer = static_cast<Color*>(malloc(93*26*2));
	fs::File logoFile = SPIFFS.open("/launcher/ByteBoiLogo.raw");
	if(logoFile){
		logoFile.read(reinterpret_cast<uint8_t*>(logoBuffer), 93 * 26 * 2);
		logoFile.close();
	}else{
		free(logoBuffer);
		logoBuffer = nullptr;
	}
	canvas->clear(C_HEX(0x0041ff));
	if(logoBuffer) canvas->drawIcon(logoBuffer, (display->getWidth() - 93) / 2, (display->getHeight() - 26) / 2, 93, 26);
	display->commit();
	delay(1000);

	auto color = canvas->readPixelRGB(0, 0);
	lgfx::rgb565_t pixel(0xffff);
	while(color.B8() > 0){
		color.set(color.R8() * 0.7, color.G8() * 0.7, color.B8() * 0.7);

		canvas->clear(C_RGB(color.R8(), color.G8(), color.B8()));
		if(logoBuffer){
			pixel.set(pixel.R8() * 0.7, pixel.G8() * 0.7, pixel.G8() * 0.7);
			Color c = pixel.operator unsigned short();
			for(int i = 0; i < 93 * 26; i++){
				if(logoBuffer[i] != TFT_TRANSPARENT){
					logoBuffer[i] = c;
				}
			}
			canvas->drawIcon(logoBuffer, (display->getWidth() - 93) / 2, (display->getHeight() - 26) / 2, 93, 26);
		}

		if(color.B8() < 3) color.set(0, 0, 0);
		display->commit();
	}
	free(logoBuffer);
	canvas->clear(TFT_BLACK);
	display->commit();
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

void ByteBoiImpl::splash(void(* callback)()){
	Color* logoBuffer = static_cast<Color*>(ps_malloc(93*26*2));
	fs::File logoFile = SPIFFS.open("/launcher/ByteBoiLogo.raw");
	if(!logoFile){
		Serial.println("Error opening splash logo");
		free(logoBuffer);
		if(callback != nullptr){
			splashCallback = nullptr;
			lastSplashDraw = 0;
			callback();
		}
		return;
	}
	logoFile.read(reinterpret_cast<uint8_t*>(logoBuffer), 93 * 26 * 2);
	logoFile.close();

	display->getBaseSprite()->clear(C_HEX(0x0041ff));
	display->getBaseSprite()->drawIcon(logoBuffer, (display->getWidth() - 93) / 2, (display->getHeight() - 26) / 2, 93, 26);
	display->commit();
	free(logoBuffer);

	splashCallback = callback;
	lastSplashDraw = millis();
	splashIndex = 0;
	LoopManager::addListener(this);

	if(splashCallback == nullptr){
		while(lastSplashDraw){
			loop(0);
		}
	}
}

void ByteBoiImpl::loop(uint micros){
	uint32_t m = millis();

	if(splashIndex > 9){
		lastSplashDraw = 0;
		splashIndex = 0;
		splashCallback = nullptr;
		LoopManager::removeListener(this);
		return;
	}else if(splashIndex == 9){
		if(m - lastSplashDraw < 1000) return;

		lastSplashDraw = 0;
		splashIndex = 0;
		LoopManager::removeListener(this);

		if(splashCallback != nullptr){
			void(*callback)() = splashCallback;
			splashCallback = nullptr;
			callback();
		}

		return;
	}

	if(m - lastSplashDraw >= 250){
		lastSplashDraw = m;
		Sprite* canvas = display->getBaseSprite();

		if(splashIndex == 8){
			auto bg = canvas->readPixelRGB(0, 0);

			for(int i = 0; i < canvas->width(); i++){
				for(int j = 0; j < canvas->height(); j++){
					auto color = canvas->readPixelRGB(i, j);
					uint8_t c = (!(color == bg)) * 255;
					color.set(c, c, c);
					canvas->fillRect(i, j, 1, 1, color.operator unsigned int());
				}
			}
		}else{
			auto bg = canvas->readPixelRGB(0, 0);
			const RGBColor& c = splashValues[splashIndex];
			int dr = (c.R8() - bg.B8()) + 256;
			int dg = (c.G8() - bg.G8()) + 256;
			int db = (c.B8() - bg.R8()) + 256;

			for(int i = 0; i < canvas->width(); i++){
				for(int j = 0; j < canvas->height(); j++){
					auto color = canvas->readPixelRGB(i, j);
					color.set((color.B8() + dr) % 256,
							  (color.G8() + dg) % 256,
							  (color.R8() + db) % 256);
					canvas->fillRect(i, j, 1, 1, color.operator unsigned int());
				}
			}
		}

		display->commit();
		splashIndex++;
	}
}


