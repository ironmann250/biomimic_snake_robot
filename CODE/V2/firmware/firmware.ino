#define ENCODER1_PIN_A 2
#define ENCODER1_PIN_B 1
#define ENCODER2_PIN_A 11
#define ENCODER2_PIN_B 12
#define RESET_COUNT_ON_BOOT 1

#define LED_PIN 7
#define PWM_2 41
#define AIN_1 38
#define AIN_2 37
#define BIN_1 36
#define BIN_2 35
#define SLEEP_PIN 39

#define HEATER1_PIN 40  
#define HEATER2_PIN 41
#include <Arduino.h>
#include "motorConfig.h"
#include "tuning.h"
#include "bleCom.h"
#include "utils.h"


MotorPID motor1, motor2;
TuneSet<> tuning;


unsigned long previousMillis = 0;
const long interval = 100;  // interval in milliseconds

unsigned long previousMillis2 = 0;
const long interval2 = 500;
float freq1 = 0.0f, freq2 = 0.0f, phase1 = 0.0f, phase2 = 0.0f, amp1 = 0.0f, amp2 = 0.0f;
int8_t mode=0, mot_enabled=0, enable_heater1=0, enable_heater2=0, enable_auto=0;

void setup() {
    Serial.begin(115200);
    //while (!Serial);
    pinMode(HEATER1_PIN, OUTPUT);
    pinMode(HEATER2_PIN, OUTPUT);
    Serial.println("starting");
    // Initialize motor system with correct pin mapping
    motorInit(
        ENCODER1_PIN_A, ENCODER1_PIN_B,
        ENCODER2_PIN_A, ENCODER2_PIN_B,
        AIN_2, AIN_1,  // Motor 1 pins (AIN_2 = 37, AIN_1 = 38)
        BIN_2, BIN_1,  // Motor 2 pins (BIN_2 = 35, BIN_1 = 36)
        SLEEP_PIN,
        RESET_COUNT_ON_BOOT,
        8344 //revs per cycle
    );
    Serial.println("starting");
    // Initialize motor controllers with explicit configurations
    motor1.init({0, AIN_2, AIN_1, 8344}); // Motor 1 (num 0) uses AIN_2 & AIN_1
    motor2.init({1, BIN_2, BIN_1, 8344}); // Motor 2 (num 1) uses BIN_2 & BIN_1
    Serial.println("starting");
    // Tuning setup
    tuning.add("tar1", motor1.Setpoint);
    tuning.add("kp1", motor1.Kp);
    tuning.add("ki1", motor1.Ki);
    tuning.add("kd1", motor1.Kd);
    tuning.add("tar2", motor2.Setpoint);
    tuning.add("kp2", motor2.Kp);
    tuning.add("ki2", motor2.Ki);
    tuning.add("kd2", motor2.Kd);
    Serial.println("starting");
    motor1.setSetpointDeg(0.0f);
    motor2.setSetpointDeg(0.0f);

    Serial.println("Zeroing out motor 2");
    printEncoder();
    do {
    motor2.update();
    printEncoder();
    }
    while (motor2.Input!=0.0f);
    Serial.println("Zeroing out motor 1");
    printEncoder();
    do {
    motor1.update();
    printEncoder(); 
    }
    while (motor1.Input!=0.0f); // Wait for motors to be initialized
    Serial.println("motors zeroed");
    printEncoder();

    // Initialize BLE
    initBLE("SnakeRobot");
    // zero out encoders
    //motor1.startSinusoidalOscillation(0.15f, 180.0f);
    //motor2.startSinusoidalOscillation(0.15f, 180.0f);
    Serial.println("Setup complete");
}

void loop() {
    // Get BLE command
    unsigned long currentMillis = millis();
    unsigned long currentMillis2 = millis();
    tuning.readSerial();
    Command cmd = updateBLE();
    //debugPrint();
    
    if (cmd.isvalid) { //mode, input, mot, heat1, heat2, auto...freq,amp, ph, pos

    //enable heaters, outside because it needs unconstrained fast reading
    digitalWrite(HEATER1_PIN, static_cast<int8_t>(cmd.bytes[3]) ? HIGH : LOW); //replace with timed func 
    digitalWrite(HEATER2_PIN, static_cast<int8_t>(cmd.bytes[4]) ? HIGH : LOW); //replace with timed func
      if (static_cast<int8_t>(cmd.bytes[1])) // if input enabled
      {
        if (!static_cast<int8_t>(cmd.bytes[5])) //auto or move by joystick
        {
        //select motor
            MotorPID& motor = (static_cast<int8_t>(cmd.bytes[2]) == 0) ? motor1 : motor2;
            if (static_cast<int8_t>(cmd.bytes[2]) == 0) {
                freq1 = cmd.floats[0]; amp1 = cmd.floats[1]; phase1 =cmd.floats[2];
            }
            else {
                freq2 = cmd.floats[0]; amp2 = cmd.floats[1]; phase2 =cmd.floats[2];
            }
            //update vals //freq, amp, ph, pos
            
            if (static_cast<int8_t>(cmd.bytes[0])) // mode 1 do sinusoidal else do positional
              {
                if (motor.isOscillating() == 0) // if not oscillating
                {
                  motor.startSinusoidalOscillation(cmd.floats[0], cmd.floats[1], cmd.floats[2]);
                }
                else
                {
                    if (motor.oscillationFrequency!=cmd.floats[0]){
                        motor.setOscillationFrequency(cmd.floats[0]);
                    }
                    
                    if (motor.oscillationAmplitude!=cmd.floats[1]){
                        motor.setOscillationAmplitude(cmd.floats[1]);
                    }
                    
                    if (motor.oscillationPhase!=cmd.floats[2]){
                        motor.setOscillationPhase(cmd.floats[2]);
                    }
                    
                }
              }
              else
              {
                motor.stopSinusoidalOscillation();
                motor.goTo(int(cmd.floats[3]));
              }

            Serial.println("Valid Packet Received:");
            Serial.print("Bytes: ");
            for (int i = 0; i < BYTE_OBJECTS_LEN; i++) {
                int8_t inp=static_cast<int8_t>(cmd.bytes[i]);
                Serial.printf("%d ", inp);
            }
            Serial.println();
            Serial.print("Floats: ");
            for (int i = 0; i < FLOAT_OBJECTS_LEN; i++) {
                Serial.println(cmd.floats[i], 3);  // Print floats with precision
            }
            //motor1.setOscillationPhase(cmd.floats[1]);
            //motor2.setOscillationPhase(cmd.floats[0]);
          }
          else //auto mode
          {
            float x=mapfloat(float(static_cast<int8_t>(cmd.bytes[6])), -128, 127, -5, 5);
            float y=mapfloat(float(static_cast<int8_t>(cmd.bytes[7])), -128, 127, -5, 5);
            if (motor1.isOscillating() == 0) // if not oscillating
                {
                  motor1.startSinusoidalOscillation(freq1, amp1, phase1);
                }
            if (motor2.isOscillating() == 0) // if not oscillating
                {
                  motor2.startSinusoidalOscillation(freq2, amp2, phase2);
                }

                if (motor1.oscillationFrequency!=x){
                  motor1.setOscillationFrequency(x);
                  }
                if (motor2.oscillationFrequency!=y){
                  motor2.setOscillationFrequency(y);
                  }

            

          }
      }
    }


    static unsigned long lastSend = 0;
    if(millis() - lastSend > 30) {
        lastSend = millis();
        
        // Create sample data (position and velocity)
        int32_t dataToSend[] = {
            static_cast<int32_t>(motor1.Input),
            static_cast<int32_t>(motor1.Setpoint),
            static_cast<int32_t>(motor2.Input),
            static_cast<int32_t>(motor2.Setpoint)
        };
        
        bleSendIntegers(dataToSend);
    }

    motor1.update();
    motor2.update();

    // Handle command based on type
    // switch(cmd.cmd) {
    //     case 't':  // Target position
    //         motor1.Setpoint += cmd.value;
    //         break;
    //     case 'y':  // Target position
    //         motor1.Setpoint -= cmd.value;
    //         break;
    //     case 'p':  // Update Kp
    //         motor2.Setpoint += cmd.value;
    //         break;
    //     case 'o':  // Update Kp
    //         motor2.Setpoint -= cmd.value;
    //         break;
    //     case 'q':  // Predefined oscillation profile
    //         if(cmd.value == 0) {
    //             // Set to 90° phase (π/2 radians), 1Hz, 30° amplitude
    //             motor1.setOscillationPhase(PI);
    //             motor1.setOscillationFrequency(1.0f);
    //             motor1.setOscillationAmplitude(30.0f);
    //             motor2.setOscillationPhase(0);
    //             motor2.setOscillationFrequency(1.0f);
    //             motor2.setOscillationAmplitude(30.0f);
    //         }
    //         cmd.cmd=0;
    //         break;
    //     case 'a':  // Predefined oscillation profile
    //         if(cmd.value == 0) {
    //             // Set to 90° phase (π/2 radians), 1Hz, 30° amplitude
    //             motor1.setOscillationPhase(0.0f);
    //             motor1.setOscillationFrequency(0.25f);
    //             motor1.setOscillationAmplitude(180.0f);
    //             motor2.setOscillationPhase(0.0f);
    //             motor2.setOscillationFrequency(0.25f);
    //             motor2.setOscillationAmplitude(180.0f);
    //         }
    //         cmd.cmd=0;
    //         break;
    //     // Add more command handlers as needed
    // }
    
    // Update both motors
     if(motor1.isOscillating()) {
        // Update motor (oscillation is handled in update())
        motor1.update();
    }
    if(motor2.isOscillating()) {
        // Update motor (oscillation is handled in update())
        motor2.update();
    }

    // Serial print statements remain unchanged
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        //motor1.update();
        //motor2.update();
        debugPrint();
    }

    if (currentMillis2 - previousMillis2 >= interval2) {
        previousMillis2 = currentMillis2;
        //motor1.update();
        //blePrintf("%d,%d\n",motor1.Setpoint,motor2.Setpoint);
    }
    
    //delay(10);
}


void debugPrint()
{
    Serial.print(motor1.Setpoint); Serial.print("\t");
    Serial.print(motor1.Input);    Serial.print("\t");
    Serial.print(motor1.Output);   Serial.print("\t");
    Serial.print(motor1.Kp);       Serial.print("\t");
    Serial.print(motor1.Ki);       Serial.print("\t");
    Serial.print(motor1.Kd);       Serial.print("\t");

    Serial.print(motor2.Setpoint); Serial.print("\t");
    Serial.print(motor2.Input);    Serial.print("\t");
    Serial.print(motor2.Output);   Serial.print("\t");
    Serial.print(motor2.Kp);       Serial.print("\t");
    Serial.print(motor2.Ki);       Serial.print("\t");
    Serial.println(motor2.Kd);
}

void printEncoder()
{
  Serial.print(motor1.Setpoint); Serial.print("\t");
  Serial.print(motor1.Input);    Serial.print("\t");
  Serial.print(motor2.Setpoint); Serial.print("\t");
  Serial.println(motor2.Input);  
}


void updateHardware() 
{
  
}