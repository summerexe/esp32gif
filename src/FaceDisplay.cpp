#include "FaceDisplay.h"
#include <Adafruit_GFX.h>

void FaceDisplay::begin() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();
}

void FaceDisplay::drawIdle(bool eyeOpen) {
    display.clearDisplay();

    int eyeY = 25;
    int eyeX1 = 35;
    int eyeX2 = 93;
    int eyeWidth = 14;
    int eyeHeight = 22;

    if(eyeOpen) {
        display.fillRoundRect(eyeX1 - eyeWidth/2, eyeY - eyeHeight/2, eyeWidth, eyeHeight, 5, WHITE);
        display.fillRoundRect(eyeX2 - eyeWidth/2, eyeY - eyeHeight/2, eyeWidth, eyeHeight, 5, WHITE);
    } else {
        display.drawLine(eyeX1 - 10, eyeY, eyeX1 + 10, eyeY, WHITE);
        display.drawLine(eyeX2 - 10, eyeY, eyeX2 + 10, eyeY, WHITE);
    }

    display.fillTriangle(63, 40, 65, 40, 64, 42, WHITE);

    display.display();
}

void FaceDisplay::drawListening() {
    display.clearDisplay();
    
    // Draw big open eyes or question mark?
    // Let's draw bigger eyes for listening
    int eyeY = 25;
    int eyeX1 = 35;
    int eyeX2 = 93;
    int eyeRadius = 12;

    display.fillCircle(eyeX1, eyeY, eyeRadius, WHITE);
    display.fillCircle(eyeX2, eyeY, eyeRadius, WHITE);

    // Small nose
    display.fillTriangle(63, 40, 65, 40, 64, 42, WHITE);

    display.display();
}

void FaceDisplay::drawWaiting() {
    display.clearDisplay();
    
    // Thinking expression: Eyes looking up/side, or maybe "..."
    int eyeY = 25;
    int eyeX1 = 35;
    int eyeX2 = 93;
    int eyeRadius = 10;

    // Eyes looked up
    display.fillCircle(eyeX1, eyeY - 5, eyeRadius, WHITE);
    display.fillCircle(eyeX2, eyeY - 5, eyeRadius, WHITE);

    // Small nose
    display.fillTriangle(63, 40, 65, 40, 64, 42, WHITE);

    // Thinking dots
    display.fillCircle(10, 10, 2, WHITE);
    display.fillCircle(18, 5, 3, WHITE);
    display.fillCircle(28, 2, 4, WHITE);

    display.display();
}

void FaceDisplay::drawSpeaking() {
    drawSmile();
}

void FaceDisplay::drawSmile() {
    display.clearDisplay();

    int eyeY = 25;
    int leftEyeX = 35;
    int rightEyeX = 93;
    int eyeWidth = 10;
    int eyeHeight = 5;
    int tilt = 2;

    for (int yOffset = 0; yOffset <= 2; yOffset++) {
        for (int x = -eyeWidth; x <= eyeWidth; x++) {
            float y = -eyeHeight * sqrt(1.0 - sq((float)x / eyeWidth)) + yOffset;
            y += (x > 0) ? -tilt : 0;
            display.drawPixel(leftEyeX + x, eyeY + y, WHITE);
        }
    }
    for (int yOffset = 0; yOffset <= 2; yOffset++) {
        for (int x = -eyeWidth; x <= eyeWidth; x++) {
            float y = -eyeHeight * sqrt(1.0 - sq((float)x / eyeWidth)) + yOffset;
            y += (x < 0) ? -tilt : 0;
            display.drawPixel(rightEyeX + x, eyeY + y, WHITE);
        }
    }

    display.fillTriangle(63, 40, 65, 40, 64, 42, WHITE);

    int mouthCenterX = 64;
    int mouthCenterY = 46;
    int mouthRadiusX = 18;
    int mouthRadiusY = 6;

    for (int i = -mouthRadiusX; i <= mouthRadiusX; i++) {
        float y = mouthRadiusY * sqrt(1.0 - sq((float)i / mouthRadiusX));
        display.drawPixel(mouthCenterX + i, mouthCenterY + y, WHITE);
    }

    int teethOffset = 4;
    display.drawLine(mouthCenterX - 18, mouthCenterY + 1 + teethOffset,
                     mouthCenterX - 15, mouthCenterY + 6 + teethOffset, WHITE);
    display.drawLine(mouthCenterX - 15, mouthCenterY + 6 + teethOffset,
                     mouthCenterX - 12, mouthCenterY + 1 + teethOffset, WHITE);

    display.drawLine(mouthCenterX + 18, mouthCenterY + 1 + teethOffset,
                     mouthCenterX + 15, mouthCenterY + 6 + teethOffset, WHITE);
    display.drawLine(mouthCenterX + 15, mouthCenterY + 6 + teethOffset,
                     mouthCenterX - 12, mouthCenterY + 1 + teethOffset, WHITE);

    display.display();
}
