#ifndef BYTEBOI_LIBRARY_BATTERYSERVICE_H
#define BYTEBOI_LIBRARY_BATTERYSERVICE_H

#include <Arduino.h>
#include <Loop/LoopListener.h>
#include <Wire.h>
#include <Display/Sprite.h>
#include <Input/InputListener.h>
#include <esp_adc_cal.h>

class BatteryService : public LoopListener{
public:
	void begin();
	void loop(uint micros) override;
	uint16_t getVoltage(bool bypassChrg = false) const;
	uint8_t getLevel() const;
	uint8_t getPercentage() const;
	void setAutoShutdown(bool enabled);
	bool isCharging() const;
	bool chargePinDetected() const;

	void drawIcon(Sprite& sprite, int16_t x, int16_t y, int16_t level = -1);

private:
	uint16_t voltage = 0; //in mV
	static constexpr uint16_t measureInterval = 2;
	static constexpr uint16_t measureCount = 10;
	uint measureMicros = 0;
	bool autoShutdown = false;
	uint8_t level = 0;
	Color* batteryBuffer[6] = {nullptr};
	uint32_t timePassed = 0;
	uint8_t pictureIndex = 0;
	float measureSum = 0;
	uint8_t measureCounter = 0;

	static constexpr uint16_t CalibRef = 624;
	int16_t calibOffset = 0;
	void calibrate();

	esp_adc_cal_characteristics_t calChars;
	bool hasChars = false;
};

#endif //BYTEBOI_LIBRARY_BATTERYSERVICE_H
