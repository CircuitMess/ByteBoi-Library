#include "PlaybackSystem.h"
#include "../Settings.h"
#include "../Pins.hpp"

PlaybackSystem Playback;

PlaybackSystem::PlaybackSystem(Sample* sample) : PlaybackSystem(){
	open(sample);
}

PlaybackSystem::PlaybackSystem() : audioTask("MixAudio", audioThread, 4 * 1024, this), queue(4, sizeof(PlaybackRequest*)){
	out = new OutputDAC(SPEAKER_PIN, SPEAKER_SD);
	updateGain();
}

PlaybackSystem::~PlaybackSystem(){
	stop();
	delete out;
}

bool PlaybackSystem::open(Sample* sample){
	stop();

	this->currentSample = sample;
	out->setSource(sample->getSource());

	return true;
}

void PlaybackSystem::play(Sample* sample){
	open(sample);
	start();
}

void PlaybackSystem::audioThread(Task* task){
	PlaybackSystem* system = static_cast<PlaybackSystem*>(task->arg);

	Serial.println("-- PlaybackSystem started --");

	while(task->running){
		PlaybackRequest* request;
		while(system->queue.count()){
			system->queue.receive(&request);

			switch(request->type){
				case PlaybackRequest::SEEK:
					system->_seek(request->value);
					break;

			}

			delete request;
		}

		if(!task->running) break;

		if(!system->schedDisabled){
			Sched.loop(0);
		}

		if(system->out->isRunning()){
			system->out->loop(0);
		}else if(system->running){
			if(system->looping){
				system->seek(0);
				system->out->start();
			}else{
				system->running = false;
			}
		}
	}

	system->running = false;
}

void PlaybackSystem::start(){
	if(running) return;
	if(!currentSample) return;

	running = true;
	out->start();
	audioTask.start(1, 0);
}

void PlaybackSystem::stop(){
	if(!running) return;

	audioTask.stop(true);
	out->stop();
	running = false;
}

bool PlaybackSystem::isRunning(){
	return running;
}

uint16_t PlaybackSystem::getDuration(){
	if(!currentSample) return 0;
	return currentSample->getSource()->getDuration();
}

uint16_t PlaybackSystem::getElapsed(){
	if(!currentSample) return 0;
	return currentSample->getSource()->getElapsed();
}

void PlaybackSystem::seek(uint16_t time) {
	if (!out->isRunning()) {
		_seek(time);
		return;
	}

	if (queue.count() == queue.getQueueSize()) return;
	PlaybackRequest *request = new PlaybackRequest({PlaybackRequest::SEEK, time});
	queue.send(&request);
}

void PlaybackSystem::setLoop(bool loop) {
	looping = loop;
}

void PlaybackSystem::_seek(uint16_t time) {
	if(!currentSample) return;

	if(out->isRunning()) {
		i2s_zero_dma_buffer((i2s_port_t) 0);
	}

	currentSample->getSource()->seek(time, SeekSet);
}

void PlaybackSystem::updateGain(){
	out->setGain((float) Settings.get().volume / 255.0f);
}

void PlaybackSystem::disableScheduler(bool schedDisabled){
	PlaybackSystem::schedDisabled = schedDisabled;
}
