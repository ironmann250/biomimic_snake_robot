#include <BLESerial.h>

// FOR ETL: Uncomment the following lines
#include <Embedded_Template_Library.h>
// #include <etl/queue.h>
#include <etl/circular_buffer.h>

String device_name = "ESP32-BLE-Slave";

//BLESerial<> SerialBLE;

// FOR ETL: Uncomment one of the following lines
// BLESerial<etl::queue<uint8_t, 255, etl::memory_model::MEMORY_MODEL_SMALL>> SerialBLE;
// OR
BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

void setup() {
    Serial.begin(115200);
    SerialBLE.begin(device_name);
}

void loop() {
    if (Serial.available()) {
        SerialBLE.write(Serial.read());
        SerialBLE.write('\n'); // Add a newline character
        SerialBLE.flush();
    }
    if (SerialBLE.available()) {
        Serial.write(SerialBLE.read());
        Serial.write('\n'); // Add a newline character
    }
}