#ifndef BYTEBOI_LIBRARY_SLIDERELEMENT_H
#define BYTEBOI_LIBRARY_SLIDERELEMENT_H

#include "SettingsElement.h"


namespace MiniMenu {
	class SliderElement : public SettingsElement {
	public:
		SliderElement(ElementContainer* partent, String name);

		void toggle() override;

		void moveSliderValue(int8_t value);

		void setSliderValue(uint8_t sliderValue);

		uint8_t getSliderValue() const;

	private:

		bool sliderIsSelected = false;

		uint8_t sliderValue = 255;


	protected:
		void drawControl() override;
	};
}


#endif //BYTEBOI_LIBRARY_SLIDERELEMENT_H
