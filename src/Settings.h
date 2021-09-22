#ifndef BYTEBOI_LIBRARY_SETTINGS_H
#define BYTEBOI_LIBRARY_SETTINGS_H

#include <Arduino.h>

struct SettingsData {
	uint32_t shutdownTime = 0;
	uint32_t sleepTime = 0;
	uint8_t volume = 20;
	bool mute = false;
	bool RGBenable = true;
};

class SettingsImpl {
public:
	bool begin();

	void store();

	SettingsData& get();

	void reset();

private:
	SettingsData data;
};

extern SettingsImpl Settings;

#endif //BYTEBOI_LIBRARY_SETTINGS_H