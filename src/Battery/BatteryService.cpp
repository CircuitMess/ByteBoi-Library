#include "BatteryService.h"
#include "../ByteBoi.h"
#include "../Bitmaps/battery_0.hpp"
#include "../Bitmaps/battery_1.hpp"
#include "../Bitmaps/battery_2.hpp"
#include "../Bitmaps/battery_3.hpp"
#include "../Bitmaps/battery_4.hpp"
#include <SPIFFS.h>
#include <Loop/LoopManager.h>

const uint16_t BatteryService::measureInterval = 1; //in seconds

void BatteryService::loop(uint micros){
	measureMicros += micros;
	if(measureMicros >= measureInterval * 200000){
		measureMicros = 0;
		analogValue += analogRead(BATTERY_PIN);
		meassureCounter++;
		if(meassureCounter == 5){
			analogValue = analogValue / 5;
			voltage = (1.1 * analogValue + 683);
			meassureCounter = 0;
			analogValue = 0;
		}
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
	if(ByteBoi.getExpander()->getPortState() & (1 << CHARGE_DETECT_PIN)){
		return ((float)voltage - (2289.61 - 0.523723*(float)voltage));
	}else{
		return voltage;
	}
}

uint8_t BatteryService::getPercentage() const{
	int16_t percentage = map(getVoltage(), 3650, 4250, 0, 100);
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

	batteryBuffer[0] = const_cast<Color*>(batteryIcon_0);
	batteryBuffer[1] = const_cast<Color*>(batteryIcon_1);
	batteryBuffer[2] = const_cast<Color*>(batteryIcon_2);
	batteryBuffer[3] = const_cast<Color*>(batteryIcon_3);
	batteryBuffer[4] = const_cast<Color*>(batteryIcon_4);

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
