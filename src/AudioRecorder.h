#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include <Arduino.h>
#include <driver/i2s.h>

// Adjust buffer size based on available RAM.
// 8kHz * 2 bytes/sample = 16000 bytes/second.
// 50,000 bytes ~= 3.1 seconds of audio.
// Reduced from 120,000 to fit in standard ESP32 RAM.
#define REC_BUFFER_SIZE 50000 

class AudioRecorder {
public:
    AudioRecorder();
    void begin(int sampleRate, const char* url);
    void start();
    void stop();
    void loop();

private:
    void configI2S();
    void sendAudio();
    void addWavHeader(uint8_t* header, int wavSize);

    int sampleRate;
    String uploadUrl;
    bool isRecording;
    
    // Large buffer to hold the entire recording
    uint8_t* audioBuffer; 
    size_t audioBufferIndex;
    
    // Temporary buffer for I2S reads
    char i2sReadBuffer[1024];
};

#endif
