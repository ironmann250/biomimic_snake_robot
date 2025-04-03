// #include <Arduino.h>
// #include "TrackEncoder.h"
// #include <ESP32MotorControl.h>
// #include <QuickPID.h> // Include QuickPID library

// #define ENCODER1_PIN_A 1
// #define ENCODER1_PIN_B 2
// #define ENCODER2_PIN_A 4
// #define ENCODER2_PIN_B 5
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
// float SetpointDegrees = 360.0f; // Desired target angle in degrees
// float Setpoint = 0.0f;          // Target encoder count
// float Input = 0.0f, Output = 0.0f, Kp = 1.0f, Ki = 0.5f, Kd = 0.1f;

// // Create the TrackEncoder object
// TrackEncoder *trackEncoder = nullptr;
// ESP32MotorControl MotorControl = ESP32MotorControl();
// QuickPID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd,
//                QuickPID::pMode::pOnError,     // Proportional mode
//                QuickPID::dMode::dOnMeas,      // Derivative mode
//                QuickPID::iAwMode::iAwClamp,   // Integral anti-windup mode
//                QuickPID::Action::direct);     // Control direction

// void setup() {
//     Serial.begin(115200);
//     while (!Serial) {}

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
//     myPID.SetMode(QuickPID::Control::automatic); // Enable automatic mode

//     Serial.println("Setup complete");
// }

// void loop() {
//     // Get encoder feedback (current encoder count)
//     Input = trackEncoder->getEncoder1Count();

//     // Compute PID
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
//     Serial.println(Output);

//     delay(10); // Short delay for stability
// }





// // Pin Definitions
// #define ENCODER1_PIN_A 1
// #define ENCODER1_PIN_B 2
// #define ENCODER2_PIN_A 11
// #define ENCODER2_PIN_B 12
// #define RESET_COUNT_ON_BOOT 1
// // Remove PULSES_PER_REV as it's defined in TrackEncoder.h
// #define LED_PIN 7
// #define PWM_2 41
// #define AIN_1 38
// #define AIN_2 37
// #define ADC_1 1
// #define ADC_2 2
// #define BIN_1 36
// #define BIN_2 35
// #define SLEEP_PIN 39
// #define BRAKING_THRESHOLD 1

// #include <Arduino.h>
// #include "TrackEncoder.h"
// #include <ESP32MotorControl.h>
// #include <QuickPID.h>
// #include "tuning.h"

// // PID Control Structure - move before any references
// struct MotorPID {
//     float Setpoint = 0.0f;
//     float Input = 0.0f;
//     float Output = 0.0f;
//     float Kp = 1.32f;
//     float Ki = 10.28f;
//     float Kd = 0.10f;
//     QuickPID pid;
    
//     MotorPID() : pid(&Input, &Output, &Setpoint, Kp, Ki, Kd,
//                    QuickPID::pMode::pOnError,
//                    QuickPID::dMode::dOnMeas,
//                    QuickPID::iAwMode::iAwClamp,
//                    QuickPID::Action::direct) {}
// };

// // Add these globals after other global declarations

// void updatePID(MotorPID &motor);
// void controlMotor(uint8_t motorNum, float output, float error);

// // Motor Control
// ESP32MotorControl MotorControl;

// // Motor instances
// MotorPID motor1, motor2;
// TuneSet<> tuning;

// // Encoder System
// TrackEncoder *trackEncoder = nullptr;

// void setup() {
//     delay(100);
//     Serial.begin(115200);
//     delay(100);
//     //while (!Serial) {}

//     // Initialize SLEEP pin
//     pinMode(SLEEP_PIN, OUTPUT);
//     digitalWrite(SLEEP_PIN, HIGH);  // Enable the motor driver initially

//     // Initialize encoder system
//     trackEncoder = new TrackEncoder(ENCODER1_PIN_A, ENCODER1_PIN_B, 
//                                   ENCODER2_PIN_A, ENCODER2_PIN_B, 
//                                   "encoderStorage");
//     if (!trackEncoder) {
//         Serial.println("Failed to initialize encoder");
//         while (1);
//     }

//     if (RESET_COUNT_ON_BOOT) {
//         trackEncoder->resetCounts();
//         Serial.println("Encoder counts reset");
//     }
//     trackEncoder->begin(200);

//     // Initialize motor driver
    

//     // Setup tuning parameters
//     tuning.add("tar1", motor1.Setpoint);
//     tuning.add("kp1", motor1.Kp);
//     tuning.add("ki1", motor1.Ki);
//     tuning.add("kd1", motor1.Kd);
    
//     tuning.add("tar2", motor2.Setpoint);
//     tuning.add("kp2", motor2.Kp);
//     tuning.add("ki2", motor2.Ki);
//     tuning.add("kd2", motor2.Kd);

//     // Initialize LED pin
//     pinMode(LED_PIN, OUTPUT);
    
//     // Initialize NimBLE with enhanced visibility settings
//     Serial.println("Dual Motor PID Controller Ready");
// }

// void updatePID(MotorPID &motor) {
//     if (motor.pid.GetKp() != motor.Kp || 
//         motor.pid.GetKi() != motor.Ki ||
//         motor.pid.GetKd() != motor.Kd) {
//         motor.pid.SetTunings(motor.Kp, motor.Ki, motor.Kd);
//     }
//     motor.pid.Compute();
// }

// void controlMotor(uint8_t motorNum, float output, float error) {
//     digitalWrite(SLEEP_PIN, HIGH);
//     if (abs(error) <= BRAKING_THRESHOLD) {
//         // Active braking when within threshold
//         MotorControl.motorStop(motorNum);
//     } else if (output > 0) {
//         MotorControl.motorForward(motorNum, abs(output));
//     } else {
//         MotorControl.motorReverse(motorNum, abs(output));
//     }
// }



// void loop() {
//     tuning.readSerial();
//     digitalWrite(SLEEP_PIN,HIGH); 
//     // Update encoder inputs
//     motor1.Input = trackEncoder->getEncoder1Count();
//     motor2.Input = trackEncoder->getEncoder2Count();

//     // Compute PID
//     updatePID(motor1);
//     updatePID(motor2);

//     // Calculate errors
//     float error1 = motor1.Setpoint - motor1.Input;
//     float error2 = motor2.Setpoint - motor2.Input;

//     // Drive motors with braking logic
//     controlMotor(1, motor1.Output, error1);
//     controlMotor(0, motor2.Output, error2);

//     // Print status to Serial
//     Serial.print(motor1.Setpoint); Serial.print("\t");
//     Serial.print(motor1.Input);    Serial.print("\t");
//     Serial.print(motor1.Output);   Serial.print("\t");
//     Serial.print(motor1.Kp);       Serial.print("\t");
//     Serial.print(motor1.Ki);       Serial.print("\t");
//     Serial.print(motor1.Kd);       Serial.print("\t");
    
//     Serial.print(motor2.Setpoint); Serial.print("\t");
//     Serial.print(motor2.Input);    Serial.print("\t");
//     Serial.print(motor2.Output);   Serial.print("\t");
//     Serial.print(motor2.Kp);       Serial.print("\t");
//     Serial.print(motor2.Ki);        Serial.print("\t");
//     Serial.println(motor2.Kd);

//     delay(10); // Maintain timing consistency
// }

