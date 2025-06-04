#include "motorConfig.h"
#include <Arduino.h>

TrackEncoder* trackEncoder = nullptr;
ESP32MotorControl motorControl;

void motorInit(int enc1A, int enc1B, int enc2A, int enc2B,
              int ain1_1, int ain1_2, int ain2_1, int ain2_2,
              int sleepPin, bool resetCounts, float pulsesPerRev) {
    trackEncoder = new TrackEncoder(enc1A, enc1B, enc2A, enc2B, 
                                   "encoderStorage", pulsesPerRev);
    trackEncoder->begin(200);
    
    if(resetCounts) {
        trackEncoder->resetCounts();
    }

    motorControl.attachMotors(ain1_1, ain1_2, ain2_1, ain2_2);
    pinMode(sleepPin, OUTPUT);
    digitalWrite(sleepPin, HIGH);
}

void MotorPID::init(const Config& config) {
    cfg = config;
    motorControl = &::motorControl;
    motorNum = config.motorNum;

    

    pid = QuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd,
                  QuickPID::pMode::pOnError,
                  QuickPID::dMode::dOnMeas,
                  QuickPID::iAwMode::iAwClamp,
                  QuickPID::Action::direct);
    pid.SetOutputLimits(-100, 100);
    pid.SetSampleTimeUs(10000);
    pid.SetMode(QuickPID::Control::automatic);
}

void MotorPID::startSinusoidalOscillation(float frequencyHz, float amplitudeDeg, 
                                        float phaseOffset, uint32_t durationMs) {
    //portENTER_CRITICAL(&parameterMux);
    oscillationFrequency = frequencyHz;
    oscillationAmplitude = amplitudeDeg;
    oscillationPhase = phaseOffset;
    oscillationDuration = durationMs;
    oscillationStartTime = millis();
    oscillationActive = true;
    //portEXIT_CRITICAL(&parameterMux);
}

void MotorPID::stopSinusoidalOscillation() {
    //portENTER_CRITICAL(&parameterMux);
    oscillationActive = false;
    //portEXIT_CRITICAL(&parameterMux);
    Setpoint = Input;
    update();
}

void MotorPID::setOscillationFrequency(float newFrequencyHz) {
    //portENTER_CRITICAL(&parameterMux);
    uint32_t currentTime = millis();
    float elapsed = (currentTime - oscillationStartTime) / 1000.0f;
    float currentPhase = 2 * PI * oscillationFrequency * elapsed + oscillationPhase;
    
    oscillationStartTime = currentTime;
    oscillationPhase = currentPhase - 2 * PI * newFrequencyHz * 
                     (currentTime - oscillationStartTime) / 1000.0f;
    oscillationFrequency = newFrequencyHz;
    //portEXIT_CRITICAL(&parameterMux);
}

void MotorPID::setOscillationAmplitude(float newAmplitudeDeg) {
    //portENTER_CRITICAL(&parameterMux);
    oscillationAmplitude = newAmplitudeDeg;
    //portEXIT_CRITICAL(&parameterMux);
}

void MotorPID::setOscillationPhase(float newPhaseOffsetRad) {
    //portENTER_CRITICAL(&parameterMux);
    oscillationPhase = newPhaseOffsetRad;
    //portEXIT_CRITICAL(&parameterMux);
}

void MotorPID::update() {
    if(oscillationActive) {
        // Capture current parameters atomically
        float currentFreq, currentAmp, currentPhase;
        uint32_t startTime, duration;
        bool active;
        
        //portENTER_CRITICAL(&parameterMux);
        currentFreq = oscillationFrequency;
        currentAmp = oscillationAmplitude;
        currentPhase = oscillationPhase;
        startTime = oscillationStartTime;
        duration = oscillationDuration;
        active = oscillationActive;
        //portEXIT_CRITICAL(&parameterMux);

        if(active) {
            uint32_t currentTime = millis();
            float elapsedSec = (currentTime - startTime) / 1000.0f;
            float angleDeg = currentAmp * sin(2 * PI * currentFreq * elapsedSec + currentPhase);
            // if (angleDeg < -3)
            // {
            //   angleDeg=0;
            // }
            setSetpointDeg(angleDeg);

            // Check duration if specified
            if(duration > 0 && (currentTime - startTime) >= duration) {
                //portENTER_CRITICAL(&parameterMux);
                oscillationActive = false;
                //portEXIT_CRITICAL(&parameterMux);
            }
        }
    }
    
    Input = (motorNum == 0) ? trackEncoder->getEncoder2Count() 
                            : trackEncoder->getEncoder1Count();
    updatePID();
    controlMotor();
}

void MotorPID::setSetpointDeg(float degrees) {
    Setpoint = (degrees * cfg.pulsesPerRev) / 360.0f;
}

void MotorPID::goTo(float var) {
    Setpoint = var;
    update();
}

void MotorPID::updatePID() {
    if(pid.GetKp() != Kp || pid.GetKi() != Ki || pid.GetKd() != Kd) {
        pid.SetTunings(Kp, Ki, Kd);
    }
    pid.Compute();
}

void MotorPID::controlMotor() {
    float error = Setpoint - Input;
    
    if(abs(error) <= BRAKING_THRESHOLD) {
        motorControl->motorStop(motorNum);
    } else if(Output > 0) {
        motorControl->motorForward(motorNum, abs(Output));
    } else {
        motorControl->motorReverse(motorNum, abs(Output));
    }
}
