// single-tap to talk, WebSocket streaming, supports silence detection

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "ServoController.h"
#include "FaceDisplay.h"
#include "WSAudioStreamer.h"   // NEW CLASS

#define TOUCH_PIN 23
#define SERVO_PITCH_PIN 16
#define SERVO_YAW_PIN   17
#define SERVO_ROLL_PIN  18

const char* WIFI_SSID = "TP-Link_2F1A";
const char* WIFI_PASS = "jfcJapW8vbXA7rf3";

ServoController servos;
FaceDisplay face;
WSAudioStreamer streamer("192.168.1.106", 3001);   // Your backend WebSocket server

bool eyeOpen = true;
unsigned long lastBlink = 0;
const unsigned long BLINK_INTERVAL = 800;

enum CatState { IDLE, LISTENING, STOPPING, WAITING, RESPONDING };
CatState state = IDLE;

bool lastTouch = false;
bool listenStarted = false;

void setup() {
    Serial.begin(115200);
    delay(300);

    pinMode(TOUCH_PIN, INPUT);

    // ====== WIFI ======
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("[WIFI] Connecting...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    Serial.println("\n[WIFI] Connected!");

    // ====== HARDWARE ======
    servos.begin(SERVO_PITCH_PIN, SERVO_YAW_PIN, SERVO_ROLL_PIN);
    face.begin();

    // ====== AUDIO STREAMER ======
    streamer.begin();  // Connect WebSocket + I2S mic + I2S speaker
}

void loop() {
    // Debounce logic
    static unsigned long lastDebounceTime = 0;
    static unsigned long pressStartTime = 0; // New
    static unsigned long stoppingStartTime = 0; // For voice hangover
    static bool buttonState = false;
    static bool lastReading = false;
    
    bool reading = digitalRead(TOUCH_PIN);

    if (reading != lastReading) {
        lastDebounceTime = millis();
    }
    lastReading = reading;

    if ((millis() - lastDebounceTime) > 50) {
        if (reading != buttonState) {
            buttonState = reading;
            
            // State change logic
            if (buttonState == HIGH) {
                 // Pressed
                 if (state == IDLE || state == RESPONDING || state == WAITING) {
                    Serial.println("[TOUCH] Hold → START listening");
                    streamer.startStreaming();
                    state = LISTENING;
                    pressStartTime = millis();
                }
            } else {
                // Released
                if (state == LISTENING) {
                    long duration = millis() - pressStartTime;
                    if (duration < 500) {
                         Serial.println("[TOUCH] Short press ignored");
                         streamer.stopStreamingNoCommit();
                         state = IDLE;
                    } else {
                        Serial.println("[TOUCH] Release → Finishing capture (Voice Hangover)...");
                        // Don't stop immediately. Wait a bit to catch end of sentence.
                        state = STOPPING;
                        stoppingStartTime = millis();
                    }
                }
            }
        }
    }

    // ============================================================
    // STOPPING State (Deprecated in VAD mode, but keep logic safe)
    // ============================================================
    if (state == STOPPING) {
        if (millis() - stoppingStartTime > 600) {
             Serial.println("[STATE] Hangover done → STOP streaming");
             streamer.stopStreaming();
             Serial.println("[STATE] Waiting for response...");
             state = WAITING;
             // Reset waiting failsafe timer logic (handled in loop)
        }
    }

    // ============================================================
    // Streaming incoming audio reply?
    // ============================================================
    // Check if we are receiving audio (isTalking)
    // Note: isTalking() is updated by WSAudioStreamer::loop()
    
    // In Hold-to-talk, we ignore interruptions usually, but let's allow it to cancel listening?
    // Actually, if AI starts talking while we are LISTENING, it means we should probably stop?
    // But in hold-to-talk, the user is king. Let's stick to simple logic.

    if (state == WAITING) {
        if (streamer.isTalking()) {
             Serial.println("[VOICE] Response STARTED");
             state = RESPONDING;
        }
        // Failsafe: If waiting too long (e.g. 5s) without response, go back to IDLE
        static unsigned long waitingStart = 0;
        if (waitingStart == 0) waitingStart = millis(); // Capture start time
        
        if (millis() - waitingStart > 5000) {
             Serial.println("[TIMEOUT] No response from AI, returning to IDLE");
             state = IDLE;
             waitingStart = 0; // Reset
        }
    } else {
        // Reset timer when not in waiting
        static unsigned long waitingStart = 0; 
    }

    if (state == RESPONDING) {
        // If silence for > 1000ms, assume done
        if (!streamer.isTalking()) {
             // Double check logic: isTalking is true if audio rx < 500ms ago
             // So if it returns false, it's been > 500ms
             Serial.println("[VOICE] Reply finished");
             state = IDLE;
        }
    }

    // ============================================================
    // FACE EXPRESSION
    // ============================================================
    if (state == IDLE) {
        if (millis() - lastBlink > BLINK_INTERVAL) {
            lastBlink = millis();
            eyeOpen = !eyeOpen;
        }
        face.drawIdle(eyeOpen);
    }
    else if (state == LISTENING || state == STOPPING) {
        face.drawListening();
    }
    else if (state == WAITING) {
        face.drawWaiting();
    }
    else if (state == RESPONDING) {
        face.drawSpeaking();
    }

    // ============================================================
    // SERVO BEHAVIOR
    // ============================================================
    if (state == LISTENING || state == STOPPING) {
        servos.setPitch(servos.pitchUp);
    } else {
        servos.setPitch(servos.pitchNeutral);
    }

    streamer.loop();
}
