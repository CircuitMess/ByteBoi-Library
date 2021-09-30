#include "Menu.h"
#include "../ByteBoi.h"
#include <Loop/LoopManager.h>
#include "../Settings.h"
#include <SPIFFS.h>
#include <FS/CompressedFile.h>

MiniMenu::Menu* MiniMenu::Menu::instance = nullptr;

MiniMenu::Menu::Menu(Context* currentContext) : Modal(*currentContext, 130, ByteBoi.inFirmware() ? 38 : 57), canvas(screen.getSprite()),
	layout(new LinearLayout(&screen, VERTICAL)), audioLayout( new LinearLayout(layout, HORIZONTAL)),
	muteText(new TextElement(audioLayout, 50, 20)),
	audioSwitch( new Switch(audioLayout)){
	instance = this;

	// TODO: rework this check, exit should appear if this is the game partition
	if(!ByteBoi.inFirmware()){
		exit = new TextElement(layout, 120, 20);
	}

	buildUI();
	audioSwitch->set(Settings.get().volume, true);
}

MiniMenu::Menu::~Menu(){

}

void MiniMenu::Menu::start(){
	selectElement(0);
	bindInput();
	audioSwitch->set(Settings.get().volume, true);
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
		instance->pop();
	});

	Input::getInstance()->setBtnPressCallback(BTN_A, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			instance->audioSwitch->toggle();
			Settings.get().volume = instance->audioSwitch->getState();
			Piezo.setMute(!Settings.get().volume);
			Piezo.tone(500, 50);
		}else{
			instance->pop();
		}
	});

	Input::getInstance()->setBtnPressCallback(BTN_UP, [](){
		if(instance == nullptr) return;
		instance->selectElement(instance->selectedElement == 0 ? 1 : 0);
		Piezo.tone(500, 50);
	});

	Input::getInstance()->setBtnPressCallback(BTN_DOWN, [](){
		if(instance == nullptr) return;
		instance->selectElement((instance->selectedElement + 1) % 2);
		Piezo.tone(500, 50);
	});

	Input::getInstance()->setBtnPressCallback(BTN_RIGHT, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			Settings.get().volume = 1;
			Piezo.setMute(!Settings.get().volume);
			if(instance->audioSwitch->getState() == false)
			{
				Piezo.tone(500, 50);
			}
			instance->audioSwitch->set(true);
		}
	});

	Input::getInstance()->setBtnPressCallback(BTN_LEFT, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			instance->audioSwitch->set(false);
			Settings.get().volume = 0;
			Piezo.setMute(!Settings.get().volume);
			Piezo.tone(500, 50);
		}
	});
}

void MiniMenu::Menu::releaseInput(){
	Input::getInstance()->removeBtnPressCallback(BTN_UP);
	Input::getInstance()->removeBtnPressCallback(BTN_DOWN);
	Input::getInstance()->removeBtnPressCallback(BTN_LEFT);
	Input::getInstance()->removeBtnPressCallback(BTN_RIGHT);
	Input::getInstance()->removeBtnPressCallback(BTN_A);
	Input::getInstance()->removeBtnPressCallback(BTN_B);
}

void MiniMenu::Menu::selectElement(uint8_t index){
	layout->reposChildren();
	audioLayout->reposChildren();
	selectedElement = index;
	selectAccum = 0;
	TextElement* selectedText = selectedElement == 0 ? muteText : exit;
	selectedX = selectedText->getX();

	if(ByteBoi.inFirmware()) return;

	exit->setColor(TFT_WHITE);
	muteText->setColor(TFT_WHITE);

	selectedText->setColor(TFT_YELLOW);
}

void MiniMenu::Menu::draw(){
	canvas->clear(TFT_TRANSPARENT);
	canvas->fillRoundRect(screen.getTotalX(), screen.getTotalY(), canvas->width(), canvas->height(), 3, C_HEX(0x004194));
	canvas->fillRoundRect(screen.getTotalX() + 2, screen.getTotalY() + 2, canvas->width() - 4, canvas->height() - 4, 3, C_HEX(0x0041ff));
	screen.draw();

}

void MiniMenu::Menu::loop(uint micros){

	selectAccum += (float) micros / 1000000.0f;
	TextElement* selectedText = muteText;
	if(!ByteBoi.inFirmware()){
		selectedText = selectedElement == 0 ? muteText : exit;
	}
	int8_t newX = selectedX + sin(selectAccum * 5.0f) * 3.0f;
	selectedText->setX(newX);
	draw();
	screen.commit();
}

void MiniMenu::Menu::buildUI(){
	layout->setWHType(CHILDREN, CHILDREN);
	layout->setPadding(5);
	layout->setGutter(5);
	layout->reflow();

	audioLayout->setWHType(PARENT, CHILDREN);
	audioLayout->addChild(muteText);
	audioLayout->addChild(audioSwitch);
	audioLayout->setPadding(3);
	audioLayout->reflow();

	layout->addChild(audioLayout);
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

	muteText->setText("Sound");
	muteText->setFont(1);
	muteText->setSize(1);
	muteText->setColor(TFT_WHITE);


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
