#include "ByteBoi.h"

ByteBoiImpl ByteBoi;

void ByteBoiImpl::begin(){
	display = new Display(128, 128, -1, 0);
	expander = new I2cExpander();

	display->begin();
	display->getBaseSprite()->clear(TFT_BLACK);
	display->commit();

	expander->begin(0x74, 4, 5);
	expander->pinMode(BL_PIN, OUTPUT);
	expander->pinWrite(BL_PIN, 1);

	input = new InputI2C(expander);
	input->preregisterButtons({ BTN_A, BTN_B, BTN_C, BTN_UP, BTN_DOWN, BTN_RIGHT, BTN_LEFT });

	Piezo.begin(BUZZ_PIN);
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
