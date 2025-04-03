#include "TrackEncoder.h"

TrackEncoder::TrackEncoder(uint8_t pinA1, uint8_t pinB1, uint8_t pinA2, uint8_t pinB2, const char *nvsNamespace, float pulsesPerRev) 
    : pulsesPerRev(pulsesPerRev){
    // Initialize preferences for NVS
    if (!preferences.begin(nvsNamespace, false)) {
        Serial.println("Failed to initialize NVS");
        while (1);
    } else {
        Serial.println("NVS initialized");
    }

    // Retrieve saved encoder counts
    encoder1Count = preferences.getLong("encoder1", 0);
    encoder2Count = preferences.getLong("encoder2", 0);

    // Initialize encoders
    ESP32Encoder::useInternalWeakPullResistors = puType::up; // Use internal pull-ups
    encoder1.attachFullQuad(pinA1, pinB1);
    encoder2.attachFullQuad(pinA2, pinB2);

    // Set the encoder counts from stored values
    encoder1.setCount(encoder1Count);
    encoder2.setCount(encoder2Count);

    // Create a queue for sending encoder counts between ISR and task
    encoderQueue = xQueueCreate(10, sizeof(int64_t) * 2);
}

TrackEncoder::~TrackEncoder() {
    preferences.end(); // Close preferences when the object is destroyed
}

void TrackEncoder::begin(uint32_t timerIntervalMs) {
    // Start the save task on the second CPU
    xTaskCreatePinnedToCore(
        saveTask,               // Task function
        "SaveEncoderTask",      // Task name
        4096,                   // Stack size
        this,                   // Parameter to pass to the task
        1,                      // Priority
        &saveTaskHandle,        // Task handle
        0                       // CPU core (1 = second CPU)
    );

    // Calculate the timer frequency and configure the timer
    const uint32_t baseFrequency = 80000000; // 80 MHz (ESP32 APB clock)
    const uint32_t prescaler = 80;           // 1 MHz (1 µs resolution)
    const uint32_t desiredFrequency = baseFrequency / prescaler;
    const uint64_t triggerPeriod = (uint64_t)timerIntervalMs * desiredFrequency / 1000; // Timer interval in ticks

    // Initialize the timer with the calculated frequency
    timer = timerBegin(desiredFrequency);
    timerAttachInterruptArg(timer, timerISR, this);
    timerAlarm(timer, triggerPeriod, true, 0); // Set the timer alarm
    timerStart(timer);
}

int64_t TrackEncoder::getEncoder1Count() {
    return encoder1.getCount();
}

int64_t TrackEncoder::getEncoder2Count() {
    return encoder2.getCount();
}

void TrackEncoder::resetCounts() {
    encoder1Count = 0;
    encoder2Count = 0;

    encoder1.setCount(0);
    encoder2.setCount(0);

    preferences.putLong("encoder1", 0);
    preferences.putLong("encoder2", 0);

    Serial.println("Encoder counts reset to zero in NVS and memory");
}

// New methods
float TrackEncoder::getEncoder1Angle() {
    return (static_cast<float>(getEncoder1Count() % PULSES_PER_REV) / PULSES_PER_REV) * 360.0;
}

float TrackEncoder::getEncoder2Angle() {
    return (static_cast<float>(getEncoder2Count() % PULSES_PER_REV) / PULSES_PER_REV) * 360.0;
}

int32_t TrackEncoder::getEncoder1Revolutions() {
    return static_cast<int32_t>(getEncoder1Count() / PULSES_PER_REV);
}

int32_t TrackEncoder::getEncoder2Revolutions() {
    return static_cast<int32_t>(getEncoder2Count() / PULSES_PER_REV);
}

void TrackEncoder::sendToPlotter() {
    // Print header only once
    if (!headerPrinted) {
        Serial.println("Encoder1_Count\tEncoder1_Angle\tEncoder2_Count\tEncoder2_Angle");
        headerPrinted = true;
    }

    // Format: Encoder1_Count\tEncoder1_Angle\tEncoder2_Count\tEncoder2_Angle
    // Serial.printf("%lld\t%.2f\t%lld\t%.2f\n", 
    //               getEncoder1Count(), getEncoder1Angle(), 
    //               getEncoder2Count(), getEncoder2Angle());
    Serial.printf("%lld\t%lld\n", getEncoder1Count(),getEncoder2Count());
}


// ISR for the timer
void IRAM_ATTR TrackEncoder::timerISR(void *arg) {
    auto *instance = static_cast<TrackEncoder *>(arg);
    int64_t counts[2] = {instance->encoder1.getCount(), instance->encoder2.getCount()};
    xQueueSendFromISR(instance->encoderQueue, &counts, NULL);
}

// Task to save encoder counts to NVS
void TrackEncoder::saveTask(void *parameter) {
    auto *instance = static_cast<TrackEncoder *>(parameter);
    int64_t counts[2];

    while (true) {
        if (xQueueReceive(instance->encoderQueue, &counts, portMAX_DELAY)) {
            instance->preferences.putLong("encoder1", counts[0]);
            instance->preferences.putLong("encoder2", counts[1]);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
