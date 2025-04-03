#ifndef BLE_COM_H
#define BLE_COM_H
#define BLE_DEBUG 0
#include <Arduino.h>
#include <Embedded_Template_Library.h>
#include <BLESerial.h>
#include <etl/circular_buffer.h>
#include <stdarg.h>

#ifndef MSG_BYTE_LEN
#define MSG_BYTE_LEN 3
#endif
#ifndef DATA_LEN
#define DATA_LEN 16  // Default: 2 integers (4 bytes each)
#endif

#define BYTE_OBJECTS_LEN 8    // Example length for bytes
#define FLOAT_OBJECTS_LEN 4   // Example length for floats

struct Command {
    bool isvalid;                               // Checksum validation flag
    uint8_t bytes[BYTE_OBJECTS_LEN];            // Array to hold byte data
    float floats[FLOAT_OBJECTS_LEN];            // Array to hold float data
    Command() : isvalid(false) {
        memset(bytes, 0, sizeof(bytes));
        memset(floats, 0, sizeof(floats));
    }
};

// Helper function to convert bytes to int32_t
inline int32_t bytesToInt(const uint8_t* bytes) {
    return static_cast<int32_t>(
        (bytes[0] << 24) | 
        (bytes[1] << 16) | 
        (bytes[2] << 8) | 
        bytes[3]
    );
}

void initBLE(const char* deviceName);
Command updateBLE();
void blePrintf(const char *fmt, ...);
void bleSendIntegers(const int32_t* values);
#endif