#include "SliderElement.h"

SettingsMiniScreen::SliderElement::SliderElement(ElementContainer* partent, String name) : SettingsElement(partent, name){

}

void SettingsMiniScreen::SliderElement::toggle(){
	sliderIsSelected = !sliderIsSelected;
}

void SettingsMiniScreen::SliderElement::moveSliderValue(int8_t value){
	sliderValue = constrain(sliderValue + 10*value, 0, 255);
}

void SettingsMiniScreen::SliderElement::setSliderValue(uint8_t sliderValue){
	SliderElement::sliderValue = sliderValue;
}

uint8_t SettingsMiniScreen::SliderElement::getSliderValue() const{
	return sliderValue;
}

void SettingsMiniScreen::SliderElement::drawControl(){
	float movingCursor;
	if(sliderValue == 0){
		movingCursor = 0;
	}else{
		movingCursor = ((float) sliderValue / 255) * 51.0f;
	}
	getSprite()->drawRect(getTotalX() + 100, getTotalY() + 12, 2, 5, TFT_WHITE);
	getSprite()->drawRect(getTotalX() + 153, getTotalY() + 12, 2, 5, TFT_WHITE);
	getSprite()->drawRect(getTotalX() + 100, getTotalY() + 14, 55, 1, TFT_WHITE);
		getSprite()->fillRoundRect(getTotalX() + 100 + movingCursor, getTotalY() + 11, 4, 7, 1, TFT_WHITE);

}
