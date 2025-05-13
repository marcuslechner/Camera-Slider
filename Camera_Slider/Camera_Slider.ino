/*
  Project: Motorized Camera Slider
  Author: Marcus Lechner
  Original Date: July 17, 2019
  Refactored: May 12, 2025

  Overview:
  A two-axis motion control system for a camera slider using NEMA 23 stepper motors 
  and TMC2208 drivers. The system supports both real-time and time-lapse modes, 
  with parameter selection via rotary encoder and feedback on a 128x32 OLED display.

  Refactor Summary:
  - Full codebase restructured for clarity, modularity, and maintainability
  - All key functions isolated, renamed, and documented
  - Rotary encoder interaction unified and debounced
  - OLED updates reduced for performance and readability
  - Motion blending logic simplified and parameterized

  Status:
  This firmware has not yet been re-tested on hardware. The slider is currently with 
  the person it was built for, so hardware validation is pending.

  Future Improvements:
  - Migrate motion sequencing to an RTOS (e.g. FreeRTOS, Zephyr) for precise timing, concurrency
  - Replace soldered breadboard setup with a custom PCB for long-term durability and compact wiring
  - Add soft limit detection, homing support, and dynamic acceleration profiles
  - Modularize UI and motor control layers for reuse across other motion systems

  Key Technologies:
  - TMC2208 stepper drivers (UART via SoftwareSerial)
  - SSD1306 OLED display (I2C)
  - Rotary encoder and push-button UI (GPIO-based)
  - Motion blending using integer multipliers (no floating point in loop)

  Communication Protocols:
  - UART: Motor driver configuration and control
  - I2C: OLED status display communication
  - GPIO: Rotary encoder, step and enable control lines
*/

// ---------------------------
// Includes and Definitions
// ---------------------------
#include <Arduino.h>
#include <TMC2208Stepper.h>
#include <U8x8lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

// Pin definitions
#define TX_LIN     2
#define RX_LIN     3
#define TX_PAN     4
#define RX_PAN     5
#define STEP_LIN   10
#define STEP_PAN   11
#define EN_LIN     6
#define EN_PAN     7
#define knobCLK    8
#define knobDT     9
#define knobBtn    A0
#define panDir     A1


// Display
U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE); 

// Motor Drivers
TMC2208Stepper linearDriver = TMC2208Stepper(RX_LIN, TX_LIN);
TMC2208Stepper panDriver = TMC2208Stepper(RX_PAN, TX_PAN);

// Motion state
long posA = 0;
long alpha = 0;
long del = 0;
float hours = 0.00;

// Encoder state
int counter = 4;
int value = 2;
int prevValue = 50;
int currentStateCLK;
int previousStateCLK;


void regularMode();
void moveMode();
void selectB();
void selectBeta();
void selectA();
void selectAlpha();
void selectSpeed();
void selectTime();
void timeLapseMode();
void selectMode();
void currentState();


void setupDisplay()
{
    u8x8.begin();
    u8x8.setPowerSave(0);
    u8x8.setFont(u8x8_font_8x13_1x2_f);
    u8x8.clear();
    u8x8.drawString(0, 0, "Starting up the");
    u8x8.drawString(0, 2, "Lechner Slider");
}

void setupDrivers()
{
    linearDriver.beginSerial(115200);
    panDriver.beginSerial(115200);
    linearDriver.push();
    panDriver.push();

    linearDriver.pdn_disable(true);
    linearDriver.I_scale_analog(false);
    linearDriver.rms_current(1100);
    linearDriver.toff(2);
    linearDriver.intpol(true);
    linearDriver.en_spreadCycle(false);
    linearDriver.mstep_reg_select(true);

    panDriver.pdn_disable(true);
    panDriver.I_scale_analog(false);
    panDriver.rms_current(1100);
    panDriver.toff(2);
    panDriver.intpol(true);
    panDriver.en_spreadCycle(false);
    panDriver.mstep_reg_select(true);

    digitalWrite(EN_LIN, LOW);
    digitalWrite(EN_PAN, LOW);
    digitalWrite(panDir, LOW);

    delay(1000);
    linearDriver.shaft(false);
}

void setupPins()
{
    pinMode(EN_LIN, OUTPUT);
    pinMode(EN_PAN, OUTPUT);
    pinMode(STEP_LIN, OUTPUT);
    pinMode(STEP_PAN, OUTPUT);
    pinMode(knobCLK, INPUT);
    pinMode(knobDT, INPUT);
    pinMode(knobBtn, INPUT);
    pinMode(panDir, OUTPUT);
}

void setup()
{
    Serial.begin(9600);
    setupDisplay();
    setupPins();
    setupDrivers();

    previousStateCLK = digitalRead(knobCLK);
    Serial.println("Setup complete.");
    delay(1000);
}

void loop() 
{
  selectMode();
}

void selectMode()
{
    const int MODE_MIN = 1;
    const int MODE_MAX = 2;

    u8x8.clear();
    u8x8.drawString(0, 0, "Select a Mode:");

    counter = 0;
    value = prevValue;
    prevValue = 0;

    while (true)
    {
        currentStateCLK = digitalRead(knobCLK);

        if (currentStateCLK != previousStateCLK)
        {
            // Encoder logic: increment or decrement based on direction
            if (digitalRead(knobDT) == currentStateCLK)
            {
                counter++;
            }
            else
            {
                counter--;
            }

            // Clamp the value between available options
            value = counter / 2;
            value = constrain(value, MODE_MIN, MODE_MAX);
            counter = value * 2;
        }

        previousStateCLK = currentStateCLK;

        if (value != prevValue)
        {
            u8x8.clearLine(2);
            u8x8.clearLine(3);
            u8x8.setCursor(0, 2);

            switch (value)
            {
                case 1:
                    u8x8.print("1. Slide and Pan");
                    break;
                case 2:
                    u8x8.print("2. Time Lapse");
                    break;
                default:
                    break;
            }

            prevValue = value;
        }

        // Confirm selection on button press
        if (digitalRead(knobBtn) == LOW)
        {
            delay(500);  // Debounce delay

            if (value == 1)
            {
                regularMode();
            }
            else if (value == 2)
            {
                timeLapseMode();
            }

            return;
        }
    }
}


void regularMode()
{
    // Step through full interactive configuration for real-time movement
    selectB();        // Set end position (linear)
    selectBeta();     // Set end angle (pan)
    selectA();        // Set start position (linear)
    selectAlpha();    // Set start angle (pan)
    selectSpeed();    // Choose motion speed
    moveMode();       // Execute calculated move
    selectMode();     // Return to mode selection menu
}


void timeLapseMode()
{
    // Step through full configuration for timelapse capture mode
    selectB();         // Set end position (linear)
    selectBeta();      // Set end angle (pan)
    selectA();         // Set start position (linear)
    selectAlpha();     // Set start angle (pan)
    selectTime();      // Choose total time for timelapse
    moveMode();        // Execute smooth timelapse motion
    selectMode();      // Return to mode selection menu
}


void currentState()
{
    // Read rotary encoder direction and update counter
    if (digitalRead(knobDT) == currentStateCLK)
    {
        counter--;
    }
    else
    {
        counter++;
    }

    // Map encoder movement to a usable value range
    value = counter / 2;

    // Clamp to expected bounds for all selector functions
    const int MIN_OPTION = 1;
    const int MAX_OPTION = 3;

    value = constrain(value, MIN_OPTION, MAX_OPTION);
    counter = value * 2; // Keep counter aligned with clamped value
}

void selectB()
{
    Serial.println("SELECTING POSITION B");

    linearDriver.microsteps(0);  // Coarse movement for positioning

    u8x8.clear();
    u8x8.drawString(0, 0, "Set End Pos:");

    value = 2;
    counter = value * 2;

    while (true)
    {
        currentStateCLK = digitalRead(knobCLK);

        if (currentStateCLK != previousStateCLK)
        {
            currentState();
        }

        previousStateCLK = currentStateCLK;

        // Forward direction
        if (value == 1)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Right");

            linearDriver.shaft(true);

            while (value == 1)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_LIN, HIGH);
                digitalWrite(STEP_LIN, LOW);

                previousStateCLK = currentStateCLK;
            }
        }

        // Reverse direction
        if (value == 3)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Left");

            linearDriver.shaft(false);

            while (value == 3)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_LIN, HIGH);
                digitalWrite(STEP_LIN, LOW);

                previousStateCLK = currentStateCLK;
            }
        }

        // Confirm selection
        if (digitalRead(knobBtn) == LOW)
        {
            posA = 0;  // Mark new reference position
            Serial.print("POSITION B SET\n");
            delay(500);
            return;
        }
    }
}

void selectBeta()
{
    Serial.println("SELECTING ANGLE BETA");

    panDriver.microsteps(16);  // Coarse angular positioning

    u8x8.clear();
    u8x8.drawString(0, 0, "Set End Angle:");

    value = 2;
    counter = value * 2;

    while (true)
    {
        currentStateCLK = digitalRead(knobCLK);

        if (currentStateCLK != previousStateCLK)
        {
            currentState();
        }

        previousStateCLK = currentStateCLK;

        // Clockwise rotation
        if (value == 1)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Clockwise");

            panDriver.shaft(false);

            while (value == 1)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_PAN, HIGH);
                digitalWrite(STEP_PAN, LOW);

                previousStateCLK = currentStateCLK;
            }
        }

        // Counter-clockwise rotation
        if (value == 3)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Anti-Clockwise");

            panDriver.shaft(true);

            while (value == 3)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_PAN, HIGH);
                digitalWrite(STEP_PAN, LOW);

                previousStateCLK = currentStateCLK;
            }
        }

        // Confirm angle set
        if (digitalRead(knobBtn) == LOW)
        {
            alpha = 0;  // End angle reference
            Serial.println("ANGLE BETA SET");
            delay(500);
            return;
        }
    }
}

void selectA()
{
    Serial.println("SELECTING POSITION A");

    linearDriver.microsteps(0);  // Coarse movement for positioning

    u8x8.clear();
    u8x8.drawString(0, 0, "Set Start Pos:");

    value = 2;
    counter = value * 2;

    while (true)
    {
        currentStateCLK = digitalRead(knobCLK);

        if (currentStateCLK != previousStateCLK)
        {
            currentState();
        }

        previousStateCLK = currentStateCLK;

        // Forward (Right)
        if (value == 1)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Right");

            linearDriver.shaft(true);

            while (value == 1)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_LIN, HIGH);
                digitalWrite(STEP_LIN, LOW);
                posA++;

                previousStateCLK = currentStateCLK;
            }
        }

        // Reverse (Left)
        if (value == 3)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Left");

            linearDriver.shaft(false);

            while (value == 3)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_LIN, HIGH);
                digitalWrite(STEP_LIN, LOW);
                posA--;

                previousStateCLK = currentStateCLK;
            }
        }

        // Confirm position
        if (digitalRead(knobBtn) == LOW)
        {
            Serial.print("POSITION A SET: ");
            Serial.println(posA);
            delay(500);
            return;
        }
    }
}

void selectAlpha()
{
    Serial.println("SELECTING ANGLE ALPHA");

    panDriver.microsteps(16);  // Coarse step control for setup

    u8x8.clear();
    u8x8.drawString(0, 0, "Set Start Angle:");

    value = 2;
    counter = value * 2;

    while (true)
    {
        currentStateCLK = digitalRead(knobCLK);

        if (currentStateCLK != previousStateCLK)
        {
            currentState();
        }

        previousStateCLK = currentStateCLK;

        // Clockwise rotation
        if (value == 1)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Clockwise");

            panDriver.shaft(false);

            while (value == 1)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_PAN, HIGH);
                digitalWrite(STEP_PAN, LOW);
                alpha++;

                previousStateCLK = currentStateCLK;
            }
        }

        // Counter-clockwise rotation
        if (value == 3)
        {
            u8x8.clearLine(2);
            u8x8.drawString(0, 2, "Anti-Clockwise");

            panDriver.shaft(true);

            while (value == 3)
            {
                currentStateCLK = digitalRead(knobCLK);
                if (currentStateCLK != previousStateCLK)
                {
                    currentState();
                }

                digitalWrite(STEP_PAN, HIGH);
                digitalWrite(STEP_PAN, LOW);
                alpha--;

                previousStateCLK = currentStateCLK;
            }
        }

        // Confirm angle set
        if (digitalRead(knobBtn) == LOW)
        {
            Serial.print("ANGLE ALPHA SET: ");
            Serial.println(alpha);

            // Normalize units for step resolution
            posA *= 128;
            alpha *= 16;

            delay(500);
            u8x8.clear();
            u8x8.drawString(0, 0, "MOVING");

            return;
        }
    }
}

void selectSpeed()
{
    Serial.println("SELECTING SPEED");

    const int SPEED_MIN = 1;
    const int SPEED_MAX = 100;

    u8x8.clear();
    u8x8.drawString(0, 0, "Select Speed:");

    counter = SPEED_MAX * 2; //starts at 100
    prevValue = 0;

    while (true)
    {
        currentStateCLK = digitalRead(knobCLK);

        if (currentStateCLK != previousStateCLK)
        {
            if (digitalRead(knobDT) == currentStateCLK)
            {
                counter++;
            }
            else
            {
                counter--;
            }

            value = constrain(counter / 2, SPEED_MIN, SPEED_MAX);
            counter = value * 2;
        }

        previousStateCLK = currentStateCLK;

        if (value != prevValue)
        {
            u8x8.clearLine(2);
            u8x8.setCursor(0, 2);
            u8x8.print(value);
            prevValue = value;
        }

        if (digitalRead(knobBtn) == LOW)
        {
            delay(500);  // Debounce
            int speedSelect = value;
            del = (100 - speedSelect) * 40;  // Delay in microseconds
            Serial.print("SPEED SELECTED: ");
            Serial.println(speedSelect);
            return;
        }
    }
}

void selectTime()
{
    Serial.println("SELECTING TIME TO LAPSE");

    const int TIME_MIN = 1;
    const int TIME_MAX = 100;
    const float TIME_STEP_HOURS = 0.25;

    u8x8.clear();
    u8x8.drawString(0, 0, "Hours to Lapse:");

    counter = 0;
    value = 0;
    prevValue = 0;

    while (true)
    {
        currentStateCLK = digitalRead(knobCLK);

        if (currentStateCLK != previousStateCLK)
        {
            if (digitalRead(knobDT) == currentStateCLK)
            {
                counter++;
            }
            else
            {
                counter--;
            }

            value = constrain(counter / 2, TIME_MIN, TIME_MAX);
            counter = value * 2;
        }

        previousStateCLK = currentStateCLK;

        if (value != prevValue)
        {
            hours = value * TIME_STEP_HOURS;

            u8x8.clearLine(2);
            u8x8.setCursor(0, 2);
            u8x8.print(hours);

            prevValue = value;
        }

        if (digitalRead(knobBtn) == LOW)
        {
            delay(500);  // Debounce

            long totalMicros = static_cast<long>(0.9 * hours * 3600000000.0);
            del = totalMicros / abs(posA);  // delay per step

            Serial.print("TOTAL HOURS: ");
            Serial.println(hours);
            Serial.print("STEP DELAY (us): ");
            Serial.println(del);
            return;
        }
    }
}

void moveMode()
{
    u8x8.clear();
    u8x8.drawString(0, 0, "Calculating...");
    u8x8.drawString(0, 2, "Please Wait...");

    linearDriver.microsteps(128);
    panDriver.microsteps(256);

    Serial.print("posA: "); Serial.println(posA);
    Serial.print("alpha: "); Serial.println(alpha);

    // Determine shaft directions
    linearDriver.shaft(posA <= 0);  // false = forward, true = reverse
    panDriver.shaft(alpha > 0);     // true = reverse (angle increases)

    posA = abs(posA);
    alpha = abs(alpha);

    long primarySteps = max(posA, alpha);
    long secondarySteps = min(posA, alpha);
    bool linearIsPrimary = (posA >= alpha);

    int multipliers[5] = {0};
    long stepsDone = 0;
    long stepsLeft = secondarySteps;

    // Compute up to 5 multipliers to approximate step blending
    for (int m = 0; m < 5 && stepsLeft > 0; ++m)
    {
        multipliers[m] = 1 + (primarySteps / stepsLeft);
        for (long i = 0; i < primarySteps; ++i)
        {
            if (i % multipliers[m] == 0)
            {
                stepsDone++;
            }
        }
        stepsLeft = secondarySteps - stepsDone;
        Serial.print("Multiplier "); Serial.print(m + 1); Serial.print(": ");
        Serial.println(multipliers[m]);
    }

    u8x8.clear();
    u8x8.drawString(0, 0, "Now Executing");
    u8x8.drawString(0, 2, "Smooth Move");

    // Execute move: primary axis steps every iteration, secondary axis steps based on multipliers
    for (long i = 0; i <= primarySteps; ++i)
    {
        bool shouldStepSecondary = false;
        for (int m = 0; m < 5; ++m)
        {
            if (multipliers[m] > 0 && i % multipliers[m] == 0)
            {
                shouldStepSecondary = true;
                break;
            }
        }

        if (linearIsPrimary)
        {
            // Step linear motor always
            digitalWrite(STEP_LIN, HIGH); digitalWrite(STEP_LIN, LOW);

            // Step pan motor conditionally
            if (shouldStepSecondary)
            {
                digitalWrite(STEP_PAN, HIGH); digitalWrite(STEP_PAN, LOW);
            }
        }
        else
        {
            // Step pan motor always
            digitalWrite(STEP_PAN, HIGH); digitalWrite(STEP_PAN, LOW);

            // Step linear motor conditionally
            if (shouldStepSecondary)
            {
                digitalWrite(STEP_LIN, HIGH); digitalWrite(STEP_LIN, LOW);
            }
        }

        delayMicroseconds(del);
    }

    Serial.println("Motion complete.");
    selectMode();  // Return to mode selection
}

