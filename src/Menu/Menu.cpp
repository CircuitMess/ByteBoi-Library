#include "Menu.h"
#include "../ByteBoi.h"
#include <Loop/LoopManager.h>
#include "../Settings.h"
#include <SPIFFS.h>
#include <FS/CompressedFile.h>

MiniMenu::Menu* MiniMenu::Menu::instance = nullptr;

MiniMenu::Menu::Menu(Context* currentContext) : Modal(*currentContext, 130, ByteBoi.isStandalone() ? 56 : 72), canvas(screen.getSprite()),
												layout(new LinearLayout(&screen, VERTICAL)), RGBEnableLayout(new LinearLayout(layout, HORIZONTAL)),
												volumeLayout(new LinearLayout(layout, HORIZONTAL)),
												ledText(new TextElement(RGBEnableLayout, 50, 20)),
												volumeText(new TextElement(volumeLayout, 50, 20)),
												LEDSwitch(new Switch(RGBEnableLayout)),
												volumeSlider(new SliderElement(volumeLayout)){
	instance = this;

	// TODO: rework this check, exit should appear if this is the game partition
	if(!ByteBoi.isStandalone()){
		exit = new TextElement(layout, 120, 20);
	}

	buildUI();
	LEDSwitch->set(Settings.get().RGBenable, true);
	volumeSlider->setSliderValue(Settings.get().volume);

	texts[0] = volumeText;
	texts[1] = ledText;
	texts[2] = exit;
}

MiniMenu::Menu::~Menu(){

}

void MiniMenu::Menu::start(){
	selectElement(0);
	bindInput();
	LEDSwitch->set(Settings.get().RGBenable, true);
	volumeSlider->setSliderValue(Settings.get().volume);
	LoopManager::addListener(this);
}

void MiniMenu::Menu::stop(){
	Settings.get().volume = instance->volumeSlider->getSliderValue();
	Settings.get().RGBenable = instance->LEDSwitch->getState();
	Settings.store();
	releaseInput();
	LoopManager::removeListener(this);

}

void MiniMenu::Menu::bindInput(){

	Input::getInstance()->setBtnPressCallback(BTN_B, [](){
		if(instance == nullptr) return;
		instance->pop();
	});

	Input::getInstance()->setBtnPressCallback(BTN_A, [](){
		if(instance == nullptr) return;
		Piezo.tone(500, 50);
		if(instance->selectedElement == 1){
			instance->LEDSwitch->toggle();
			Settings.get().RGBenable = instance->LEDSwitch->getState();
		}else if(instance->selectedElement == 2){
			instance->stop();
			ByteBoi.backToLauncher();
		}
	});

	Input::getInstance()->setBtnPressCallback(BTN_UP, [](){
		if(instance == nullptr) return;
		instance->selectedElement--;
		if(instance->selectedElement < 0){
			instance->selectedElement = instance->layout->getChildren().size() - 1;
		}
		instance->selectElement(instance->selectedElement);
		Piezo.tone(500, 50);
	});

	Input::getInstance()->setBtnPressCallback(BTN_DOWN, [](){
		if(instance == nullptr) return;
		instance->selectedElement++;
		if(instance->selectedElement >= instance->layout->getChildren().size()){
			instance->selectedElement = 0;
		}
		instance->selectElement(instance->selectedElement);
		Piezo.tone(500, 50);
	});

	Input::getInstance()->setBtnPressCallback(BTN_RIGHT, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			instance->volumeSlider->moveSliderValue(1);
		}else if(instance->selectedElement == 1){
			instance->LEDSwitch->toggle();
			Settings.get().RGBenable = instance->LEDSwitch->getState();
		}
	});

	Input::getInstance()->setBtnPressCallback(BTN_LEFT, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			instance->volumeSlider->moveSliderValue(-1);
		}else if(instance->selectedElement == 1){
			instance->LEDSwitch->toggle();
			Settings.get().RGBenable = instance->LEDSwitch->getState();
		}
	});
	Input::getInstance()->setBtnPressCallback(BTN_C, [](){
		if(instance == nullptr) return;
		instance->pop();
	});
}

void MiniMenu::Menu::releaseInput(){
	Input::getInstance()->removeBtnPressCallback(BTN_UP);
	Input::getInstance()->removeBtnPressCallback(BTN_DOWN);
	Input::getInstance()->removeBtnPressCallback(BTN_LEFT);
	Input::getInstance()->removeBtnPressCallback(BTN_RIGHT);
	Input::getInstance()->removeBtnPressCallback(BTN_A);
	Input::getInstance()->removeBtnPressCallback(BTN_B);
	Input::getInstance()->removeBtnPressCallback(BTN_C);
}

void MiniMenu::Menu::selectElement(uint8_t index){
	layout->reposChildren();
	RGBEnableLayout->reposChildren();
	volumeLayout->reposChildren();
	selectedElement = index;
	selectAccum = 0;

	ledText->setColor(TFT_WHITE);
	volumeText->setColor(TFT_WHITE);

	if(exit){
		exit->setColor(TFT_WHITE);
	}

	texts[index]->setColor(TFT_YELLOW);
	selectedX = texts[index]->getX();
}

void MiniMenu::Menu::draw(){
	canvas->clear(TFT_TRANSPARENT);
	canvas->fillRoundRect(screen.getTotalX(), screen.getTotalY(), canvas->width(), canvas->height(), 3, C_HEX(0x004194));
	canvas->fillRoundRect(screen.getTotalX() + 2, screen.getTotalY() + 2, canvas->width() - 4, canvas->height() - 4, 3, C_HEX(0x0041ff));
    Battery.drawIcon(*canvas,111,4);
    screen.draw();

}

void MiniMenu::Menu::loop(uint micros){
	if(LEDSwitch->getState()){
		ledsBlinking = true;
	}else{
		ledsBlinking = false;
		LED.setRGB(OFF);
		previousLEDState = LEDSwitch->getState();
		LEDcounter = 0;
	}
	if(ledsBlinking){
		Serial.println(LEDcounter);
		Serial.println(previousLEDState);
		Serial.println(previousLEDTime);
		if(LEDSwitch->getState() && !previousLEDState){
			if(millis() - previousLEDTime >= 100){
				previousLEDTime = millis();
				if(LED.getRGB() == OFF){
					LED.setRGB(static_cast<LEDColor>(LEDColor::WHITE));
					LEDcounter++;
				}else{
					LED.setRGB(OFF);
				}
				if(LEDcounter >= 10){
					LEDcounter = 0;
					previousLEDTime = 0;
					previousLEDState = LEDSwitch->getState();
				}
			}
		}else if(!LEDSwitch->getState() && previousLEDState){
			LED.setRGB(OFF);
			previousLEDState = LEDSwitch->getState();
			previousLEDTime = 0;
		}
	}
	selectAccum += (float) micros / 1000000.0f;
	int8_t newX = selectedX + sin(selectAccum * 5.0f) * 3.0f;
	texts[selectedElement]->setX(newX);
	draw();
	screen.commit();
}

void MiniMenu::Menu::buildUI(){
	layout->setWHType(PARENT, CHILDREN);
	layout->setPadding(7);
	layout->reflow();

	RGBEnableLayout->setWHType(PARENT, CHILDREN);
	RGBEnableLayout->addChild(ledText);
	RGBEnableLayout->addChild(LEDSwitch);
	RGBEnableLayout->setPadding(1);
	RGBEnableLayout->reflow();

	volumeLayout->setWHType(PARENT,CHILDREN);
	volumeLayout->addChild(volumeText);
	volumeLayout->addChild(volumeSlider);
	volumeLayout->reflow();

	layout->addChild(volumeLayout);
	layout->addChild(RGBEnableLayout);
	if(exit){
		layout->addChild(exit);
		exit->setFont(1);
		exit->setSize(1);
		exit->setColor(TFT_WHITE);
		exit->setAlignment(TextElement::CENTER);
		exit->setText("Exit Game");
	}

	layout->reflow();
	screen.addChild(layout);
	screen.repos();

	ledText->setText("LEDs");
	ledText->setFont(1);
	ledText->setSize(1);
	ledText->setColor(TFT_WHITE);

	volumeText->setText("Volume");
	volumeText->setFont(1);
	volumeText->setSize(1);
	volumeText->setColor(TFT_WHITE);
}

void MiniMenu::Menu::returned(void* data){
	prevModal = (Modal*)data;
}

void MiniMenu::Menu::popIntoPrevious(){
	ModalTransition* transition = static_cast<ModalTransition*>((void*)instance->pop());
	if(instance->prevModal == nullptr) return;
	transition->setDoneCallback([](Context* currContext, Modal*){
		instance->prevModal->push(currContext);
	});
//		instance->pop();
}
