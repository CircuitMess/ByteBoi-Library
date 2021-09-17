#ifndef BYTEBOI_LIBRARY_BYTEBOILED_H
#define BYTEBOI_LIBRARY_BYTEBOILED_H

#include <Arduino.h>

enum WLEDColor {
		OFF, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
};
class ByteBoiLED {
public:
	void begin();
	void setRGB(WLEDColor colour);
	WLEDColor getRGB();
private:
	WLEDColor currentColor;

};

extern ByteBoiLED LED;

#endif //BYTEBOI_LIBRARY_BYTEBOILED_H
