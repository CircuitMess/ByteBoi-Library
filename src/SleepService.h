#ifndef BYTEBOI_LIBRARY_SLEEPSERVICE_H
#define BYTEBOI_LIBRARY_SLEEPSERVICE_H

#include <Loop/LoopListener.h>
#include <Input/InputListener.h>

class SleepService : public LoopListener, public InputListener {
public:
	void loop(uint) override;
private:
	uint autoShutdownMicros = 0;
	uint32_t lastShutdownTime = (uint32_t)-1;
	void anyKeyPressed() override;
};

extern SleepService Sleep;
#endif //BYTEBOI_LIBRARY_SLEEPSERVICE_H
