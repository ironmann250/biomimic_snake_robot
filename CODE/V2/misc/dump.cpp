// #include <Arduino.h>
// #include "TrackEncoder.h"
// #include <ESP32MotorControl.h>
// #include <QuickPID.h> // Include QuickPID library
// #include "tuning.h"

// #define ENCODER1_PIN_A 1
// #define ENCODER1_PIN_B 2
// #define ENCODER2_PIN_A 11
// #define ENCODER2_PIN_B 12
// #define RESET_COUNT_ON_BOOT 1
// #define PULSES_PER_REV 8344
// #define LED_PIN 7
// #define PWM_2 41
// #define AIN_1 38
// #define AIN_2 37
// #define ADC_1 1
// #define ADC_2 2
// #define BIN_1 36
// #define BIN_2 35
// #define SLEEP_PIN 39
// #define SPI_MOSI 11
// #define SPI_MISO 13
// #define SPI_SCK 12
// #define CS1 10
// #define CS2 14
// #define CS3 21

// // Variables for PID control
// float SetpointDegrees = 0.0f; // Desired target angle in degrees
// float Setpoint = 0.0f;          // Target encoder count
// float Input = 0.0f, Output = 0.0f, Kp = 1.32f, Ki = 10.28f, Kd = 0.10f;  //kp=1.32,ki=10.28,kd=0.10 at 5v usb
// float prevKp, prevKi, prevKd;

// // Create the TrackEncoder object
// TrackEncoder *trackEncoder = nullptr;
// ESP32MotorControl MotorControl = ESP32MotorControl();
// QuickPID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd,
//                QuickPID::pMode::pOnError,     // Proportional mode
//                QuickPID::dMode::dOnMeas,      // Derivative mode
//                QuickPID::iAwMode::iAwClamp,   // Integral anti-windup mode
//                QuickPID::Action::direct);     // Control direction
// TuneSet<> tuning;

// void setup() {
//     delay(100);
//     Serial.begin(115200);
//     delay(100);
//     while (!Serial) {}
//     tuning.add("tar", Setpoint);    
//     tuning.add("kp", Kp);                             
//     tuning.add("ki", Ki);
//     tuning.add("kd", Kd);
//     // Initialize the TrackEncoder system
//     trackEncoder = new TrackEncoder(ENCODER1_PIN_A, ENCODER1_PIN_B, ENCODER2_PIN_A, ENCODER2_PIN_B, "encoderStorage");

//     if (!trackEncoder) {
//         Serial.println("Failed to initialize TrackEncoder");
//         while (1);
//     }
    
//     if (RESET_COUNT_ON_BOOT) {
//         trackEncoder->resetCounts();
//         Serial.println("Encoder counts have been reset");
//     }
    
//     trackEncoder->begin(200);
//     MotorControl.attachMotors(AIN_1, AIN_2, BIN_2, BIN_1);
//     pinMode(SLEEP_PIN, OUTPUT);

//     // Convert setpoint from degrees to encoder counts
//     Setpoint = (SetpointDegrees * PULSES_PER_REV) / 360.0f;

//     // Configure PID controller
//     myPID.SetOutputLimits(-100, 100);   // Limit output to PWM range
//     myPID.SetSampleTimeUs(10000);      // Set the sample time to 10 ms
//     myPID.SetTunings(Kp, Ki, Kd);      // Set the PID tunings
//     prevKp = Kp;
//     prevKi = Ki;
//     prevKd = Kd;
//     myPID.SetMode(QuickPID::Control::automatic); // Enable automatic mode

//     Serial.println("Setup complete");
// }

// void loop() {
//     // Get encoder feedback (current encoder count)
//     tuning.readSerial();
//     Input = trackEncoder->getEncoder1Count();
//     // Compute PID
//     if (Kp != prevKp || Ki != prevKi || Kd != prevKd) {
//         myPID.SetTunings(Kp, Ki, Kd);
//         prevKp = Kp;
//         prevKi = Ki;
//         prevKd = Kd;
//         //Serial.println("PID tunings updated");
//     }

//     myPID.Compute();
    
//     // Apply PID output to control motor
//     digitalWrite(SLEEP_PIN, HIGH);
//     if (Output > 0) {
//         MotorControl.motorForward(1, abs(Output));
//     } else {
//         MotorControl.motorReverse(1, abs(Output));
//     }

//     // Print PID values for debugging and Arduino Serial Plotter
//     // Format: "Setpoint\tInput\tOutput"
//     Serial.print(Setpoint);
//     Serial.print("\t");
//     Serial.print(Input);
//     Serial.print("\t");
//     Serial.print(Output);
//     Serial.print("\t");
//     Serial.print(Kp);
//     Serial.print("\t");
//     Serial.print(Ki);
//     Serial.print("\t");
//     Serial.print(Kd);
//     Serial.print("\t");
//     Serial.println(trackEncoder->getEncoder2Count());
//     delay(10); // Short delay for stability
// }
