#ifndef BYTEBOI_LIBRARY_BATTERYSERVICE_H
#define BYTEBOI_LIBRARY_BATTERYSERVICE_H

#include <Arduino.h>
#include <Loop/LoopListener.h>
#include <Wire.h>
#include <Display/Sprite.h>
#include <Input/InputListener.h>

class BatteryService : public LoopListener{
public:
	BatteryService()= default;

	void begin();
	void loop(uint micros) override;
	uint16_t getVoltage() const;
	uint8_t getLevel() const;
	uint8_t getPercentage() const;
	void disableShutdown(bool _shutdown);
	bool isCharging() const;

	void drawIcon(Sprite& sprite, int16_t x, int16_t y);

private:
	uint16_t voltage = 0; //in mV
	static const uint16_t measureInterval;
	uint measureMicros = measureInterval*1000000;
	bool shutdownDisable = false;
	uint8_t level = 0;
	Color* batteryBuffer[6] = {nullptr};
	uint32_t timePassed = 0;
	uint8_t pictureIndex = 0;
	uint16_t maxVoltage = 0;
	float analogValue = 0;
	uint8_t meassureCounter = 0;
};

#endif //BYTEBOI_LIBRARY_BATTERYSERVICE_H
