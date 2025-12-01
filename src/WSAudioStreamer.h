#ifndef WS_AUDIO_STREAMER_H
#define WS_AUDIO_STREAMER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

class WSAudioStreamer {
public:
    WSAudioStreamer(const char* host, uint16_t port);

    void begin();
    void loop();

    void startStreaming();
    void stopStreaming();
    void stopStreamingNoCommit(); // New
    
    void sendCommit(); // New function

    bool detectedSilence();
    bool isTalking(); // Replaces isPlayingReply with time-based check
    bool isConnected();

private:
    void configI2S();
    
    const char* _host;
    uint16_t _port;
    
    bool _streaming;
    bool _playing;
    
    WebSocketsClient ws;

    // Audio settings
    static const int SAMPLE_RATE = 16000; // Fixed for OpenAI Realtime usually
    static const int SILENCE_THRESHOLD = 60; // Increased from 20
    static const unsigned long SILENCE_DURATION = 3000; // ms

    unsigned long lastSoundTime;
    unsigned long _lastRxTime = 0;
    uint8_t i2sReadBuffer[1024];
};

#endif
