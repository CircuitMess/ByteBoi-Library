#include "ByteBoi.h"
#include <SPIFFS.h>

ByteBoiImpl ByteBoi;

void ByteBoiImpl::begin(){
	display = new Display(160, 120, -1, 1);
	expander = new I2cExpander();

	display->begin();
	display->getBaseSprite()->clear(TFT_BLACK);
	display->commit();

	expander->begin(0x74, 23, 22);
	expander->pinMode(BL_PIN, OUTPUT);
	expander->pinWrite(BL_PIN, 0);

	input = new InputI2C(expander);
	input->preregisterButtons({ BTN_A, BTN_B, BTN_C, BTN_UP, BTN_DOWN, BTN_RIGHT, BTN_LEFT });

	Piezo.begin(BUZZ_PIN);
}

void ByteBoi::setDataRoot(String dataRoot){
	ByteBoi::dataRoot = dataRoot;
}

void ByteBoi::open(String path, const char* mode){
	SPIFFS.open(String(dataRoot + path), mode);
}
Display* ByteBoiImpl::getDisplay(){
	return display;
}

I2cExpander* ByteBoiImpl::getExpander(){
	return expander;
}

InputI2C* ByteBoiImpl::getInput(){
	return input;
}
