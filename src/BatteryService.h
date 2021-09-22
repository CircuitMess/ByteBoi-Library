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

private:
	uint16_t voltage = 0; //in mV
	static const uint16_t measureInterval;
	uint measureMicros = measureInterval*1000000;
	bool shutdownDisable = false;
	uint8_t level = 0;
};


#endif //BYTEBOI_LIBRARY_BATTERYSERVICE_H
