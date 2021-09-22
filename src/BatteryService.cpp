#include "BatteryService.h"
#include "ByteBoi.h"
#include <Support/ContextTransition.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Loop/LoopManager.h>

const uint16_t BatteryService::measureInterval = 1; //in seconds

void BatteryService::loop(uint micros){
	measureMicros += micros;
	if(measureMicros >= measureInterval * 1000000){
		measureMicros = 0;
		float x = analogRead(BATTERY_PIN);
		voltage = (1.3255 * x - 1.2461);
		if(getLevel() == 0 && !shutdownDisable){
			ByteBoi.getExpander()->pinMode(BL_PIN, 1);
			WiFi.mode(WIFI_OFF);
			btStop();
			esp_deep_sleep_start();
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
}

bool BatteryService::isCharging() const{
	return (ByteBoi.getExpander()->getPortState() & (1 << CHARGE_DETECT_PIN));
}
