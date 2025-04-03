#ifndef TRACK_ENCODER_H
#define TRACK_ENCODER_H

#include <ESP32Encoder.h>
#include <Preferences.h>
#include <Arduino.h>
#include <esp32-hal-timer.h>

class TrackEncoder {
public:
     TrackEncoder(uint8_t pinA1, uint8_t pinB1, uint8_t pinA2, uint8_t pinB2, const char *nvsNamespace, float pulsesPerRev);
    ~TrackEncoder();

    void begin(uint32_t timerIntervalMs);
    int64_t getEncoder1Count();
    int64_t getEncoder2Count();
    void resetCounts();
    
    // New methods
    float getEncoder1Angle();
    float getEncoder2Angle();
    int32_t getEncoder1Revolutions();
    int32_t getEncoder2Revolutions();
    void sendToPlotter();


private:
    float pulsesPerRev;
    ESP32Encoder encoder1, encoder2;
    Preferences preferences;

    hw_timer_t *timer = nullptr;
    TaskHandle_t saveTaskHandle;
    QueueHandle_t encoderQueue;

    volatile int64_t encoder1Count = 0;
    volatile int64_t encoder2Count = 0;

    static constexpr int PULSES_PER_REV = 8344; // Calculated as 7 PPR * 4 (quadrature) * 298 (gear ratio)

    static void IRAM_ATTR timerISR(void *arg);
    static void saveTask(void *parameter);
    bool headerPrinted = false; // Flag to ensure header is printed only once
};

#endif // TRACK_ENCODER_H
