//  touch-and-hold to talk, HTTP POST method, no supports silence detection

#include <Arduino.h>
#include <WiFi.h>
#include "ServoController.h"
#include "FaceDisplay.h"
#include "AudioRecorder.h"

// ================= CONFIG ==================
#define TOUCH_PIN 23
#define SERVO_PITCH_PIN 16
#define SERVO_YAW_PIN   17
#define SERVO_ROLL_PIN  18

// WIFI CREDENTIALS - REPLACE THESE
const char* WIFI_SSID = "TP-Link_2F1A";
const char* WIFI_PASS = "jfcJapW8vbXA7rf3";

// ================= GLOBAL MODULES ==========
ServoController servos;
FaceDisplay face;
AudioRecorder recorder;

// ================= STATE ===================
bool eyeOpen = true;
unsigned long lastBlink = 0;
const unsigned long BLINK_INTERVAL = 800;

enum CatState { IDLE, SMILE };
CatState currentState = IDLE;

unsigned long smileStart = 0;
const unsigned long SMILE_DURATION = 1500;

bool wasTouched = false;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== BOOTING ESP32 CAT ROBOT ===");

    pinMode(TOUCH_PIN, INPUT);

    // ====== WIFI ======
    Serial.println("[WIFI] Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WIFI] Connected!");
        Serial.print("[WIFI] IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[WIFI] Failed to connect. Check credentials.");
    }

    // ====== SERVOS ======
    Serial.println("[SERVOS] Initializing...");
    servos.begin(SERVO_PITCH_PIN, SERVO_YAW_PIN, SERVO_ROLL_PIN);
    Serial.println("[SERVOS] Ready.");

    // ====== OLED ======
    Serial.println("[OLED] Initializing...");
    face.begin();
    Serial.println("[OLED] Ready.");

    // ====== AUDIO ======
    Serial.println("[AUDIO] Initializing recorder...");
    // NOTE: Ensure this IP matches your computer's IP
    // Reduced to 8000 Hz to save memory and allow longer recording time (approx 3s)
    recorder.begin(8000, "http://192.168.1.106:3000/upload-audio");
    Serial.println("[AUDIO] recorder.begin() finished.");
}

void loop() {
    // ==============================
    // TOUCH SENSOR (start/stop audio)
    // ==============================
    bool touched = digitalRead(TOUCH_PIN);

    if (touched && !wasTouched) {
        Serial.println("[TOUCH] detected → START recording");
        recorder.start();
    }
    if (!touched && wasTouched) {
        Serial.println("[TOUCH] released → STOP recording");
        recorder.stop();
    }
    wasTouched = touched;

    // ==============================
    // SERVO MOVEMENT
    // ==============================
    if (touched) {
        currentState = SMILE;
        smileStart = millis();
        servos.setPitch(servos.pitchUp);
        servos.setYaw(servos.yawRight);
        servos.setRoll(servos.rollRight);
    } else {
        servos.setPitch(servos.pitchNeutral);
        servos.setYaw((servos.yawLeft + servos.yawRight) / 2);
        servos.setRoll((servos.rollLeft + servos.rollRight) / 2);
    }

    // ==============================
    // FACE DISPLAY
    // ==============================
    if (currentState == SMILE && millis() - smileStart > SMILE_DURATION) {
        currentState = IDLE;
    }

    if (currentState == IDLE && millis() - lastBlink > BLINK_INTERVAL) {
        lastBlink = millis();
        eyeOpen = !eyeOpen;
    }

    if (currentState == IDLE) face.drawIdle(eyeOpen);
    else face.drawSmile();

    // ==============================
    // AUDIO LOOP
    // ==============================
    recorder.loop();

    delay(20);
}
