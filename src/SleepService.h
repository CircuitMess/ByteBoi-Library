#ifndef BYTEBOI_LIBRARY_SLEEPSERVICE_H
#define BYTEBOI_LIBRARY_SLEEPSERVICE_H

#include <Loop/LoopListener.h>
#include <Input/InputListener.h>
#include <Input/InputI2C.h>

class SleepService : public LoopListener, public InputListener {
public:
	void loop(uint) override;

	void begin(InputI2C* input);
	void stop(InputI2C* input);
private:
	uint autoShutdownMicros = 0;
	uint32_t lastShutdownTime = (uint32_t)-1;
	void anyKeyPressed() override;
};

extern SleepService Sleep;
#endif //BYTEBOI_LIBRARY_SLEEPSERVICE_H
