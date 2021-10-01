#include "Menu.h"
#include "../ByteBoi.h"
#include <Loop/LoopManager.h>
#include "../Settings.h"
#include <SPIFFS.h>
#include <FS/CompressedFile.h>

MiniMenu::Menu* MiniMenu::Menu::instance = nullptr;

MiniMenu::Menu::Menu(Context* currentContext) : Modal(*currentContext, 130, ByteBoi.inFirmware() ? 61 : 80), canvas(screen.getSprite()),
												layout(new LinearLayout(&screen, VERTICAL)), RGBEnableLayout(new LinearLayout(layout, HORIZONTAL)),
												volumeLayout(new LinearLayout(layout, HORIZONTAL)),
												muteText(new TextElement(RGBEnableLayout, 50, 20)),
												volumeText(new TextElement(volumeLayout, 50, 20)),
												LEDSwitch(new Switch(RGBEnableLayout)),
												volumeSlider(new SliderElement(RGBEnableLayout)){
	instance = this;

	// TODO: rework this check, exit should appear if this is the game partition
	if(!ByteBoi.inFirmware()){
		exit = new TextElement(layout, 120, 20);
	}

	buildUI();
	LEDSwitch->set(Settings.get().RGBenable, true);
	volumeSlider->setSliderValue(Settings.get().volume);
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
	Settings.store();
	releaseInput();
	LoopManager::removeListener(this);

}

void MiniMenu::Menu::bindInput(){

	Input::getInstance()->setBtnPressCallback(BTN_B, [](){
		if(instance == nullptr) return;
		Settings.get().volume = instance->volumeSlider->getSliderValue();
		Settings.get().RGBenable = instance->LEDSwitch->getState();
		instance->pop();
	});

	Input::getInstance()->setBtnPressCallback(BTN_A, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			instance->LEDSwitch->toggle();
			Settings.get().volume = instance->LEDSwitch->getState();
			Piezo.setMute(!Settings.get().volume);
			Piezo.tone(500, 50);
		}else if(instance->selectedElement == 2){
			Settings.get().volume = instance->volumeSlider->getSliderValue();
			Settings.get().RGBenable = instance->LEDSwitch->getState();
			instance->pop();
		}
	});

	Input::getInstance()->setBtnPressCallback(BTN_UP, [](){
		if(instance == nullptr) return;
		instance->selectedElement--;
		if(instance->selectedElement < 0){
			instance->selectedElement = 2;
		}
		instance->selectElement(instance->selectedElement);

		Piezo.tone(500, 50);
	});

	Input::getInstance()->setBtnPressCallback(BTN_DOWN, [](){
		if(instance == nullptr) return;
		instance->selectedElement++;
		if(instance->selectedElement > 2){
		instance->selectedElement = 0;
	}
		instance->selectElement(instance->selectedElement);
		Piezo.tone(500, 50);
	});

	Input::getInstance()->setBtnPressCallback(BTN_RIGHT, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			Settings.get().volume = 1;
			Piezo.setMute(!Settings.get().volume);
			if(instance->LEDSwitch->getState() == false)
			{
				Piezo.tone(500, 50);
			}
			instance->LEDSwitch->set(true);
		}
		if(instance->selectedElement == 1){
			instance->volumeSlider->moveSliderValue(1);
		}
	});

	Input::getInstance()->setBtnPressCallback(BTN_LEFT, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			instance->LEDSwitch->set(false);
			Settings.get().volume = 0;
			Piezo.setMute(!Settings.get().volume);
			Piezo.tone(500, 50);
		}
		if(instance->selectedElement == 1){
			instance->volumeSlider->moveSliderValue(-1);
		}
	});
	Input::getInstance()->setBtnPressCallback(BTN_C, [](){
		if(instance == nullptr) return;
		Settings.get().volume = instance->volumeSlider->getSliderValue();
		Settings.get().RGBenable = instance->LEDSwitch->getState();
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
	TextElement* selectedText ;
	if(selectedElement == 0){
		selectedText = muteText;
	}else if(selectedElement == 1){
		selectedText = volumeText;
	}else if(selectedElement == 2){
		selectedText = exit;
	}
	selectedX = selectedText->getX();

	if(ByteBoi.inFirmware()) return;

	exit->setColor(TFT_WHITE);
	muteText->setColor(TFT_WHITE);
	volumeText->setColor(TFT_WHITE);

	selectedText->setColor(TFT_YELLOW);
}

void MiniMenu::Menu::draw(){
	canvas->clear(TFT_TRANSPARENT);
	canvas->fillRoundRect(screen.getTotalX(), screen.getTotalY(), canvas->width(), canvas->height(), 3, C_HEX(0x004194));
	canvas->fillRoundRect(screen.getTotalX() + 2, screen.getTotalY() + 2, canvas->width() - 4, canvas->height() - 4, 3, C_HEX(0x0041ff));
	RGBEnableLayout->getChild(0)->draw();
    Battery.drawIcon(*canvas,110,70);
    screen.draw();

}

void MiniMenu::Menu::loop(uint micros){

	selectAccum += (float) micros / 1000000.0f;
	TextElement* selectedText;
	if(selectedElement == 0){
		selectedText = muteText;
	}else if(selectedElement == 1){
		selectedText = volumeText;
	}else if(selectedElement == 2){
		selectedText = exit;
	}
	int8_t newX = selectedX + sin(selectAccum * 5.0f) * 3.0f;
	selectedText->setX(newX);
	draw();
	screen.commit();
}

void MiniMenu::Menu::buildUI(){
	layout->setWHType(PARENT, CHILDREN);
	layout->setPadding(7);
	layout->reflow();

	RGBEnableLayout->setWHType(PARENT, CHILDREN);
	RGBEnableLayout->addChild(muteText);
	RGBEnableLayout->addChild(LEDSwitch);
	RGBEnableLayout->setPadding(3);
	RGBEnableLayout->reflow();

	volumeLayout->setWHType(PARENT,CHILDREN);
	volumeLayout->addChild(volumeText);
	volumeLayout->addChild(volumeSlider);
	volumeLayout->reflow();

	layout->addChild(RGBEnableLayout);
	layout->addChild(volumeLayout);
	if(!ByteBoi.inFirmware()){
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

	muteText->setText("LEDs");
	muteText->setFont(1);
	muteText->setSize(1);
	muteText->setColor(TFT_WHITE);

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
