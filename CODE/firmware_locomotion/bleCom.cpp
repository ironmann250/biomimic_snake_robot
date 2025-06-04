#include "bleCom.h"

BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

// Packet parsing state machine
// Packet constants
constexpr uint8_t HEADER = 0xA5;
constexpr uint8_t FOOTER = 0x5A;

void initBLE(const char* deviceName) {
    SerialBLE.begin(deviceName);
}


Command updateBLE() {
    Command result;
    static enum {
        WAIT_HEADER,
        COLLECT_BYTES,
        COLLECT_FLOATS,
        GET_CHECKSUM,
        CHECK_FOOTER
    } state = WAIT_HEADER;

    static uint8_t byteIndex = 0;
    static uint8_t floatByteIndex = 0;
    static uint8_t floatBuffer[4];  // Buffer to collect bytes for a single float
    static uint8_t checksum = 0;
    static uint8_t receivedChecksum = 0;

    while (SerialBLE.available()) {
        uint8_t c = SerialBLE.read();

        switch (state) {
            case WAIT_HEADER:
                if (c == 0xA5) {  // HEADER
                    state = COLLECT_BYTES;
                    byteIndex = 0;
                    floatByteIndex = 0;
                    checksum = 0;
                }
                break;

            case COLLECT_BYTES:
                if (byteIndex < BYTE_OBJECTS_LEN) {
                    result.bytes[byteIndex++] = c;
                    checksum += c;
                }
                if (byteIndex == BYTE_OBJECTS_LEN) {
                    state = COLLECT_FLOATS;
                }
                break;

            case COLLECT_FLOATS:
                if (floatByteIndex < FLOAT_OBJECTS_LEN * 4) {
                    floatBuffer[floatByteIndex % 4] = c;
                    checksum += c;
                    floatByteIndex++;
                    if ((floatByteIndex % 4) == 0) {
                        // Convert 4 bytes to a float
                        uint32_t floatInt = ((uint32_t)floatBuffer[3] << 24) |
                                            ((uint32_t)floatBuffer[2] << 16) |
                                            ((uint32_t)floatBuffer[1] << 8) |
                                            ((uint32_t)floatBuffer[0]);
                        float convertedFloat;
                        memcpy(&convertedFloat, &floatInt, sizeof(float));
                        result.floats[(floatByteIndex / 4) - 1] = convertedFloat;
                    }
                }
                if (floatByteIndex == FLOAT_OBJECTS_LEN * 4) {
                    state = GET_CHECKSUM;
                }
                break;

            case GET_CHECKSUM:
                receivedChecksum = c;
                state = CHECK_FOOTER;
                break;

            case CHECK_FOOTER:
                if (c == 0x5A) {  // FOOTER
                    checksum &= 0xFF;
                    result.isvalid = (checksum == receivedChecksum);
                }
                state = WAIT_HEADER;  // Reset for next packet
                break;
        }
    }

    return result;
}


void bleSendIntegers(const int32_t* values) {
    const uint8_t numIntegers = DATA_LEN / 4;
    const uint8_t packetSize = 3 + DATA_LEN;
    uint8_t packet[packetSize];
    
    // Header
    packet[0] = 0xA5;
    
    // Pack data bytes in little-endian format
    uint8_t checksum = 0;
    for(uint8_t i = 0; i < numIntegers; i++) {
        uint32_t val = static_cast<uint32_t>(values[i]);
        // Little-endian byte order
        packet[1 + i*4] = val & 0xFF;          // LSB
        packet[2 + i*4] = (val >> 8) & 0xFF;
        packet[3 + i*4] = (val >> 16) & 0xFF;
        packet[4 + i*4] = (val >> 24) & 0xFF;  // MSB
        
        // Update checksum with all 4 bytes
        for(uint8_t j = 0; j < 4; j++) {
            checksum += packet[1 + i*4 + j];
        }
    }

    // Add checksum and footer
    packet[1 + DATA_LEN] = checksum & 0xFF;
    packet[2 + DATA_LEN] = 0x5A;
    if (BLE_DEBUG)
    {
        // Debug output (now shows both formats)
        Serial.printf("[%lu] Sending BLE packet: ", millis());
        for(uint8_t i = 0; i < packetSize; i++) {
            Serial.printf("%02X ", packet[i]);
        }
        Serial.printf("\nRaw values: ");
        for(uint8_t i = 0; i < numIntegers; i++) {
            Serial.printf("%d (0x%08X) ", values[i], values[i]);
        }
        Serial.printf("\nLittle-endian bytes:\n");
        for(uint8_t i = 0; i < numIntegers; i++) {
            Serial.printf("  Value %d: %02X %02X %02X %02X\n", i+1,
                        packet[1 + i*4],     // LSB
                        packet[2 + i*4],
                        packet[3 + i*4],
                        packet[4 + i*4]);    // MSB
        }
        Serial.printf("Checksum: %02X\n\n", checksum & 0xFF);
    }
    // Send packet
    SerialBLE.write(packet, packetSize);
    SerialBLE.flush();
}





void blePrintf(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    for(int i = 0; buf[i] != '\0'; i++) {
        SerialBLE.write(buf[i]);
        SerialBLE.flush();
    }
}