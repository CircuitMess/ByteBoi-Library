#include "BatteryService.h"
#include "../ByteBoi.h"
#include <SPIFFS.h>
#include <Loop/LoopManager.h>

const uint16_t BatteryService::measureInterval = 1; //in seconds

void BatteryService::loop(uint micros){
	measureMicros += micros;
	if(measureMicros >= measureInterval * 1000000){
		measureMicros = 0;
		float x = analogRead(BATTERY_PIN);
		voltage = (1.1 * x + 683);
		if(getLevel() == 0 && !shutdownDisable && !isCharging()){
			ByteBoi.shutdown();
			return;
		}
		measureMicros = 0;
	}
}

uint8_t BatteryService::getLevel() const{
	uint8_t percentage = getPercentage();
	if(percentage > 80){
		return 4;
	}else if(percentage <= 80 && percentage > 40){
		return 3;
	}else if(percentage <= 40 && percentage > 15){
		return 2;
	}else if(percentage <= 15 && percentage > 0){
		return 1;
	}else if(percentage == 0){
		return 0;
	}
}

uint16_t BatteryService::getVoltage() const{
	return voltage;
}

uint8_t BatteryService::getPercentage() const{
	int16_t percentage = map(voltage, 3650, 4200, 0, 100);
	if(percentage < 0){
		return 0;
	}else if(percentage > 100){
		return 100;
	}else{
		return percentage;
	}
}

void BatteryService::disableShutdown(bool _shutdown){
	shutdownDisable = _shutdown;
}

void BatteryService::begin(){
	LoopManager::addListener(this);
	ByteBoi.getExpander()->pinMode(CHARGE_DETECT_PIN, INPUT_PULLDOWN);
	pinMode(BATTERY_PIN, INPUT);

	char filename[50];
	for(uint8_t i = 0; i < 5; i++){
		batteryBuffer[i] = static_cast<Color*>(ps_malloc(14 * 6 * 2));
		sprintf(filename, "/launcher/battery_%d.raw", i);

		fs::File file = SPIFFS.open(filename);
		if(!file){
			printf("Failed opening battery icon: %s\n", filename);
			free(batteryBuffer[i]);
			batteryBuffer[i] = nullptr;
			file.close();
			continue;
		}
		file.read(reinterpret_cast<uint8_t*>(batteryBuffer[i]), 14 * 6 * 2);
		file.close();
	}
}

bool BatteryService::isCharging() const{
	if(getLevel() == 4){
		return false;
	}else{
		return (ByteBoi.getExpander()->getPortState() & (1 << CHARGE_DETECT_PIN));
	}
}

void BatteryService::drawIcon(Sprite &sprite, int16_t x, int16_t y){
	Color* buffer = batteryBuffer[getLevel()];
	if(buffer == nullptr) return;
	if(!isCharging() && timePassed != 0){
		timePassed = 0;
	}
	if(isCharging()){
		if(timePassed == 0){
			timePassed = millis();
			pictureIndex = 0;
		}
		if(millis() - timePassed >= 300){
			timePassed = millis();
			pictureIndex++;
			if(pictureIndex > 4){
				pictureIndex = 0;
			}
		}
		sprite.drawIcon(batteryBuffer[pictureIndex], x, y, 14, 6, 1, TFT_TRANSPARENT);
	}else{
		sprite.drawIcon(buffer, x, y, 14, 6, 1, TFT_TRANSPARENT);
	}
}
