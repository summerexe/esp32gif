#ifndef FACE_DISPLAY_H
#define FACE_DISPLAY_H

#include <Adafruit_SSD1306.h>

class FaceDisplay {
public:
    void begin();
    void drawIdle(bool eyeOpen);
    void drawListening(); 
    void drawWaiting(); // New
    void drawSpeaking();
    void drawSmile();

private:
    Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);
};

#endif
