#ifndef BYTEBOI_LIBRARY_BATTERYPOPUPSERVICE_H
#define BYTEBOI_LIBRARY_BATTERYPOPUPSERVICE_H

#include <Loop/LoopListener.h>

class WarningPopup;
class ShutdownPopup;

class BatteryPopupService : public LoopListener{
public:
	void loop(uint time) override;
	void enablePopups(bool enable);

private:
	static const uint16_t checkInterval;
	uint checkMicros = 0;

	static ShutdownPopup *shutdownPopup;
	static WarningPopup *warningPopup;
	bool warningShown = false;
	bool enabled = false;
};

extern BatteryPopupService BatteryPopup;
#endif //BYTEBOI_LIBRARY_BATTERYPOPUPSERVICE_H
