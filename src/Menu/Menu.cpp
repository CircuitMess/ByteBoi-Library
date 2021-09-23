#include "Menu.h"
#include "../ByteBoi.h"
#include <Loop/LoopManager.h>
#include "../Settings.h"
#include <SPIFFS.h>
#include <FS/CompressedFile.h>

Menu* Menu::instance = nullptr;

Menu::Menu(Context* currentContext) : Modal(*currentContext, 130, ByteBoi.inFirmware() ? 38 : 57), canvas(screen.getSprite()),
	layout(new LinearLayout(&screen, VERTICAL)), audioLayout( new LinearLayout(layout, HORIZONTAL)),
	muteText(new TextElement(audioLayout, 50, 20)),
	audioSwitch( new Switch(audioLayout)){
	instance = this;

	if(!ByteBoi.inFirmware()){
		exit = new TextElement(layout, 120, 20);
	}
	backgroundBuffer = static_cast<Color*>(ps_malloc(160 * 128 * 2));
	if(backgroundBuffer == nullptr){
		Serial.printf("MainMenu background picture unpack error\n");
		return;
	}

	fs::File backgroundFile = CompressedFile::open(SPIFFS.open("/launcher/background.raw.hs"), 13, 12);

	backgroundFile.read(reinterpret_cast<uint8_t*>(backgroundBuffer), 160 * 120 * 2);
	backgroundFile.close();

	buildUI();
	audioSwitch->set(Settings.get().mute, true);
}
Menu::~Menu(){
	free(backgroundBuffer);
}

void Menu::start(){
	selectElement(0);
	bindInput();
	audioSwitch->set(Settings.get().mute, true);
	LoopManager::addListener(this);
}

void Menu::stop(){
	releaseInput();
	LoopManager::removeListener(this);
	Settings.store();

}

void Menu::bindInput(){

	Input::getInstance()->setBtnPressCallback(BTN_A, [](){
		if(instance == nullptr) return;
		if(instance->selectedElement == 0){
			instance->audioSwitch->toggle();
			Settings.get().mute = instance->audioSwitch->getState();
			Piezo.setMute(!Settings.get().mute);
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
			Settings.get().mute = true;
			Piezo.setMute(!Settings.get().mute);
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
			Settings.get().mute = 0;
			Piezo.setMute(!Settings.get().mute);
			Piezo.tone(500, 50);
		}
	});
}

void Menu::releaseInput(){
	Input::getInstance()->removeBtnPressCallback(BTN_UP);
	Input::getInstance()->removeBtnPressCallback(BTN_DOWN);
	Input::getInstance()->removeBtnPressCallback(BTN_LEFT);
	Input::getInstance()->removeBtnPressCallback(BTN_RIGHT);
	Input::getInstance()->removeBtnPressCallback(BTN_A);
}

void Menu::selectElement(uint8_t index){
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

void Menu::draw(){
	canvas->clear(TFT_TRANSPARENT);
	canvas->drawIcon(backgroundBuffer, 0, 0, 160, 120, 1);
	canvas->fillTriangle(5, 0, 0, 5, 0, 0, TFT_TRANSPARENT);
	canvas->fillTriangle(canvas->width(), 0, canvas->width(), 5, canvas->width() - 5, 0, TFT_TRANSPARENT);
	canvas->fillTriangle(canvas->width(), canvas->height(), canvas->width(), canvas->height() - 5, canvas->width() - 5, canvas->height(), TFT_TRANSPARENT);
	canvas->fillTriangle(5, canvas->height(), 0, canvas->height() - 5, 0, canvas->height(), TFT_TRANSPARENT);
	screen.draw();

}

void Menu::loop(uint micros){

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

void Menu::buildUI(){
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

void Menu::returned(void* data){
	prevModal = (Modal*)data;
}

void Menu::popIntoPrevious(){
	ModalTransition* transition = static_cast<ModalTransition*>((void*)instance->pop());
	if(instance->prevModal == nullptr) return;
	transition->setDoneCallback([](Context* currContext, Modal*){
		instance->prevModal->push(currContext);
	});
//		instance->pop();
}
