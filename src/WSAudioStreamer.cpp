#include "WSAudioStreamer.h"

// Pin Definitions
#define I2S_WS      25
#define I2S_SCK     33
#define I2S_SD_IN   32
#define I2S_SD_OUT  22 // Assumed speaker pin

WSAudioStreamer::WSAudioStreamer(const char* host, uint16_t port) {
    _host = host;
    _port = port;
    _streaming = false;
    _playing = false;
    lastSoundTime = 0;
    _lastRxTime = 0;
}

void WSAudioStreamer::begin() {
    configI2S();

    ws.begin(_host, _port, "/");
    ws.onEvent([this](WStype_t type, uint8_t * payload, size_t length) {
        switch(type) {
            case WStype_CONNECTED:
                Serial.println("[WS] Connected");
                break;
            case WStype_DISCONNECTED:
                Serial.println("[WS] Disconnected");
                break;
            case WStype_TEXT:
                Serial.printf("[WS] RX Text: %s\n", payload);
                // If text starts with TXT:, update lastRxTime so robot acts like it's "talking"
                if (strncmp((char*)payload, "TXT:", 4) == 0) {
                     _lastRxTime = millis();
                }
                break;
            case WStype_BIN:
                // Received audio data from server -> Write to Speaker
                Serial.println("[WS] RX Audio Chunk"); // Debug log
                _lastRxTime = millis();

                size_t bytesWritten;
                i2s_write(I2S_NUM_0, payload, length, &bytesWritten, portMAX_DELAY);
                break;
        }
    });

    ws.setReconnectInterval(2000);
}

void WSAudioStreamer::configI2S() {
    i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
        .sample_rate = 24000, // OpenAI Realtime often uses 24kHz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // or RIGHT
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false
    };

    i2s_pin_config_t pins = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_SD_OUT,
        .data_in_num = I2S_SD_IN
    };

    i2s_driver_install(I2S_NUM_0, &config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pins);
    i2s_zero_dma_buffer(I2S_NUM_0);
}

void WSAudioStreamer::startStreaming() {
    if (!ws.isConnected()) {
        Serial.println("[WS] Not connected");
        return;
    }
    Serial.println("[WS] Sending START (Clear Buffer)");
    ws.sendTXT("START");
    // Small delay to allow flush?
    delay(10);

    _streaming = true;
    lastSoundTime = millis();
    Serial.println("[AUDIO] Start streaming");
}

void WSAudioStreamer::stopStreaming() {
    _streaming = false;
    Serial.println("[AUDIO] Stop streaming");
    sendCommit();
}

void WSAudioStreamer::stopStreamingNoCommit() {
    _streaming = false;
    Serial.println("[AUDIO] Stop streaming (No Commit)");
    // Do not send COMMIT. Backend will eventually timeout or just ignore the partial buffer.
    // Ideally we send a "CANCEL" message to clear buffer again?
    if (ws.isConnected()) {
        ws.sendTXT("CANCEL"); // Let's add this to be clean
    }
}

void WSAudioStreamer::sendCommit() {
    Serial.println("[DEBUG] Entering sendCommit...");
    if (ws.isConnected()) {
        Serial.println("[WS] Sending COMMIT");
        ws.sendTXT("COMMIT");
    } else {
        Serial.println("[WS] Cannot send COMMIT - Disconnected");
    }
}

bool WSAudioStreamer::detectedSilence() {
    if (!_streaming) return false;
    // If time since last significant sound > duration
    return (millis() - lastSoundTime > SILENCE_DURATION);
}

bool WSAudioStreamer::isTalking() {
    // Consider us "talking" if we received audio data less than 500ms ago
    return (millis() - _lastRxTime < 500); 
}

bool WSAudioStreamer::isConnected() {
    return ws.isConnected();
}

void WSAudioStreamer::loop() {
    ws.loop();

    if (_streaming && ws.isConnected()) {
        size_t bytesRead = 0;
        i2s_read(I2S_NUM_0, i2sReadBuffer, sizeof(i2sReadBuffer), &bytesRead, 0);

        if (bytesRead > 0) {
            // 1. Send to WebSocket
            ws.sendBIN(i2sReadBuffer, bytesRead);

            // 2. Check silence (Use Peak Amplitude)
            int16_t* samples = (int16_t*)i2sReadBuffer;
            int numSamples = bytesRead / 2;
            int32_t peak = 0;
            for (int i=0; i<numSamples; i++) {
                int16_t val = abs(samples[i]);
                if (val > peak) peak = val;
            }

            // Debug audio levels occasionally
            static unsigned long lastDebugTime = 0;
            if (millis() - lastDebugTime > 200) { 
                Serial.printf("[AUDIO] Peak: %d\n", peak);
                lastDebugTime = millis();
            }

            if (peak > SILENCE_THRESHOLD) {
                lastSoundTime = millis();
            }
        }
    }
}
