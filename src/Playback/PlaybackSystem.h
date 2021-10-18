#ifndef BYTEBOI_LIBRARY_PLAYBACKSYSTEM_H
#define BYTEBOI_LIBRARY_PLAYBACKSYSTEM_H


#include <CMAudio.h>
#include <Audio/OutputI2S.h>
#include <Audio/SpeedModifier.h>
#include <Audio/EffectProcessor.h>
#include <Audio/Mixer.h>
#include <Audio/SourceWAV.h>
#include <Audio/EffectType.hpp>
#include <Audio/SourceAAC.h>
#include <Audio/OutputDAC.h>
#include <Util/Task.h>
#include <Sync/Queue.h>
#include "Sample.h"

struct PlaybackRequest {
	enum { SEEK } type;
	size_t value;
};

class PlaybackSystem {
public:
	PlaybackSystem();
	PlaybackSystem(Sample* sample);
	virtual ~PlaybackSystem();

	bool open(Sample* sample);
	void play(Sample* sample);
	void start();
	void stop();
	bool isRunning();

	Task audioTask;
	static void audioThread(Task* task);

	uint16_t getDuration();
	uint16_t getElapsed();

	void seek(uint16_t time);

	void disableScheduler(bool schedDisabled);
	void setLoop(bool loop);
	void updateGain();

private:
	bool running = false;
	Queue queue;
	bool looping = false;
	bool schedDisabled = false;

	OutputDAC* out;

	Sample* currentSample = nullptr;

	void _seek(uint16_t time);
};

extern PlaybackSystem Playback;

#endif //BYTEBOI_LIBRARY_PLAYBACKSYSTEM_H
