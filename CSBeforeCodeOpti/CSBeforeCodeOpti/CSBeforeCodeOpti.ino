//Marcus Lechner
//Help from mahmud
//

#define TX_LIN    2  // BLUE, RES
#define RX_LIN    3  // GREEN
#define TX_PAN    4  // BLUE, RES
#define RX_PAN    5  // GREEN
#define STEP_LIN  10 // Step on rising edge
#define STEP_PAN  11 // Step on rising edge
#define knobCLK   8
#define knobDT    9
#define EN_LIN    6  // LOW: Driver enabled. HIGH: Driver disabled
#define EN_PAN    7  // LOW: Driver enabled. HIGH: Driver disabled

#include <TMC2208Stepper.h>
#include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include` <Adafruit_SH1106.h>
//U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE); 
//#define OLED_RESET 4
//Adafruit_SH1106 display(4);

// Create driver that uses SoftwareSerial for communication
TMC2208Stepper linearDriver = TMC2208Stepper(RX_LIN, TX_LIN);
TMC2208Stepper panDriver = TMC2208Stepper(RX_PAN, TX_PAN);
int speedSelect = 0;
int del = 0;
long counter = 4;
long value = 2;
int currentStateCLK;
int previousStateCLK;
long i = 0;
long posA = 0;
long alpha = 0;

long stepsLeft = 0;
long stepsDone = 0;
int multiplier1 = 0;
int multiplier2 = 0;
int multiplier3 = 0;
int multiplier4 = 0;
int multiplier5 = 0;

long stepLin = 0;
long stepPan = 0;

void regularMode();
void moveMode();
void selectB();
void selectBeta();
void selectA();
void selectAlpha();
void selectSpeed();
//void selectTime();
void setup() 
{
  
  //Wire.begin();
  //display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  Serial.begin(9600);
  linearDriver.beginSerial(115200);
  panDriver.beginSerial(115200);
  // Push at the start of setting up the driver resets the register to default
  linearDriver.push();
  panDriver.push();
  // Prepare pins

  pinMode(EN_LIN, OUTPUT);
  pinMode(EN_PAN, OUTPUT);
  pinMode(STEP_LIN, OUTPUT);
  pinMode(STEP_PAN, OUTPUT);
  pinMode (knobCLK, INPUT);
  pinMode (knobDT, INPUT);
  pinMode (A0, INPUT);


  //LINEAR STEPPER
  linearDriver.pdn_disable(true);     // Use PDN/UART pin for communication
  linearDriver.I_scale_analog(false); // Use internal voltage reference
  linearDriver.rms_current(1100);      // Set linearDriver current = 500mA, 0.5 multiplier for hold current and RSENSE = 0.11.
  linearDriver.toff(2);               // Enable driver in software
  linearDriver.intpol (true);         //enables interpolation
  linearDriver.microsteps (256);        //0 - 256
  linearDriver.en_spreadCycle (false); //Spread Cycle enabled. offers smooth and quiet operation
  linearDriver.mstep_reg_select(true);  //Microsteps set by the registers

  //PAN STEPPER
  panDriver.pdn_disable(true);     // Use PDN/UART pin for communication
  panDriver.I_scale_analog(false); // Use internal voltage reference
  panDriver.rms_current(1100);      // Set panDriver current = 500mA, 0.5 multiplier for hold current and RSENSE = 0.11.
  panDriver.toff(2);               // Enable driver in software
  panDriver.intpol (true);
  panDriver.microsteps (256); // @128mS 6680 steps per 360
  panDriver.en_spreadCycle (false);
  panDriver.mstep_reg_select(true);

  digitalWrite(EN_LIN, LOW);    // Enable driver in hardware
  digitalWrite(EN_PAN, LOW); 
  digitalWrite(A1, LOW);
  digitalWrite(A1, LOW);
  delay(3000);

  linearDriver.shaft(false);
  //panDriver.shaft(true);

  previousStateCLK = digitalRead(knobCLK);
  Serial.println("Set up Done!");

  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r); 
}

void loop() 
{
  linearDriver.microsteps (0); 
  speedSelect = 100;
  del = ((100 - speedSelect) * 2) + 5;
  regularMode();
}
void moveMode()
{
  u8x8.clear();
  u8x8.drawString(0,0,"CALCULATING...");
  multiplier1 = 0;
  multiplier2 = 0;
  multiplier3 = 0;
  multiplier4 = 0;
  multiplier5 = 0;
  
  linearDriver.microsteps (128);
  panDriver.microsteps (256);
  //posA = 1000000/divide;
  //alpha = 88564;
  Serial.println(posA);
  Serial.println(alpha);
  if(posA > 0)
  {
    linearDriver.shaft(false);
  }
  else
  {
    linearDriver.shaft(true);
  }


  if(alpha > 0)
  {
    Serial.println(alpha);
    panDriver.shaft(false);
    Serial.println("SHAFT FALSE");
  }
  else
  {
    Serial.println(alpha);
    panDriver.shaft(true);
    Serial.println("SHAFT TRUE");    
  }
  posA = abs(posA);
  alpha = abs(alpha);
  stepsLeft = alpha;
  //Serial.print("Actual Multiplier: "); Serial.println((((float)abs(posA )) / ((float)alpha)));

  if(stepsLeft > 0)
  {
    multiplier1 = (1 + (posA / alpha));
    Serial.print("Multiplier1: ");Serial.println(multiplier1);
    //stepsLeft = posA;
    for(i = 0; i < posA; i++)
    {
      if(i % multiplier1 == 0)
      {
        stepsDone++;
      }
    }
    stepsLeft = stepsLeft - stepsDone;
    Serial.print("STEPS DONE: "); Serial.println(stepsDone);
    Serial.print("STEPS LEFT: "); Serial.println(stepsLeft);
  }

  if(stepsLeft > 0)
  {
    multiplier2 = (1 + (posA / stepsLeft)); 
    Serial.print("Multiplier2: ");Serial.println(multiplier2);
    for(i = 0; i < posA; i++)
    {
      if(i % multiplier2 == 0)
      {
        stepsDone++;
      }
    }
    stepsLeft = alpha - stepsDone;
    Serial.print("STEPS DONE3: "); Serial.println(stepsDone); 
    Serial.print("STEPS LEFT3: "); Serial.println(stepsLeft);
  }
  
  if(stepsLeft > 0)
  {
    multiplier3 = (1 + (posA / stepsLeft)); 
    Serial.print("Multiplier3: ");Serial.println(multiplier3);
    for(i = 0; i < posA; i++)
    {
      if(i % multiplier3 == 0)
      {
        stepsDone++;
      }
    }
    stepsLeft = alpha - stepsDone;
    Serial.print("STEPS DONE4: "); Serial.println(stepsDone); 
    Serial.print("STEPS LEFT4: "); Serial.println(stepsLeft);
  }

  if(stepsLeft > 0)
  {
    multiplier4 = (1 + (posA / stepsLeft)); 
    Serial.print("Multiplier4: ");Serial.println(multiplier4);
    for(i = 0; i < posA; i++)
    {
      if(i % multiplier4 == 0)
      {
        stepsDone++;
      }
    }
    stepsLeft = alpha - stepsDone;
    Serial.print("STEPS DONE5: "); Serial.println(stepsDone); 
    Serial.print("STEPS LEFT5: "); Serial.println(stepsLeft);
  }

  if(stepsLeft > 0)
  {
    multiplier5 = (1 + (posA / stepsLeft)); 
    Serial.print("Multiplier5: ");Serial.println(multiplier5);
    for(i = 0; i < posA; i++)
    {
      if(i % multiplier5 == 0)
      {
        stepsDone++;
      }
    }
    stepsLeft = alpha - stepsDone;
    Serial.print("STEPS DONE2: "); Serial.println(stepsDone); 
    Serial.print("STEPS LEFT2: "); Serial.println(stepsLeft);
  }
  //int multiplier3 = (1 + (posA/(alpha - ((alpha/multiplier2)*(((float)abs(posA )) / ((float)alpha))))));
  //Serial.print("Multiplier3: ");Serial.println(multiplier3);
    
    
  u8x8.clear();
  u8x8.drawString(0,0,"MOVING");
  
  for(i = 0; i <= posA; i++)
  {

    while((i%multiplier1 != 0)&&(i % multiplier2 != 0)&&(i % multiplier3 != 0)&&(i % multiplier4 != 0)&&(i % multiplier5 != 0)&&(i <= posA))
    {
      delay(0);
      digitalWrite(STEP_LIN, HIGH);
      digitalWrite(STEP_LIN, LOW);
      stepLin++;
      i++;
    }
    if(i%multiplier1 == 0)
    {
      //Serial.println(1);
      digitalWrite(STEP_PAN, HIGH);
      digitalWrite(STEP_PAN, LOW);
      stepPan++;
    }
    if(i%multiplier2 == 0)
    {
      //Serial.println(2);
      digitalWrite(STEP_PAN, HIGH);
      digitalWrite(STEP_PAN, LOW);
      stepPan++;
    }
    if(i%multiplier3 == 0)
    {
      //Serial.println(3);
      digitalWrite(STEP_PAN, HIGH);
      digitalWrite(STEP_PAN, LOW);
      stepPan++;
    }
    if(i%multiplier4 == 0)
    {
      //Serial.println(4);
      digitalWrite(STEP_PAN, HIGH);
      digitalWrite(STEP_PAN, LOW);
      stepPan++;
    }
    if(i%multiplier5 == 0)
    {
      //Serial.println(5);
      digitalWrite(STEP_PAN, HIGH);
      digitalWrite(STEP_PAN, LOW);
      stepPan++;
    }

    digitalWrite(STEP_LIN, HIGH);
    digitalWrite(STEP_LIN, LOW);
    stepLin++;
    delay(0);
    
  }
  //Serial.println(i);
  Serial.println("DONE");
  Serial.print("LINEAR STEPS NEEDED: "); Serial.print(posA);Serial.print("   LINEAR STEPS DONE: "); Serial.println(stepLin);
  Serial.print("PAN STEPS NEEDED: "); Serial.print(alpha);Serial.print("   PAN STEPS DONE: "); Serial.println(stepPan);
  //while(1);
  return(0);
}

void regularMode()
{
  selectB();
  selectBeta();
  selectA();
  selectAlpha();
  selectSpeed();
  moveMode();
}
void selectB()
{
  Serial.println("SELECTING POSITION B");
  u8x8.clear();
  u8x8.drawString(0,0,"SELECT POS B");
  while(1)
  {
    currentStateCLK = digitalRead(knobCLK);
     
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }
      //Serial.print("Value: ");  Serial.println(value);
    }
    previousStateCLK = currentStateCLK;
    
  //FORWARD
    if(value == 1)
    {
        u8x8.clear();
        u8x8.drawString(0,0,"FORWARD");
    }
    while(value == 1)
    {
      linearDriver.shaft(true);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        if(digitalRead(knobDT) == currentStateCLK)
        {
            counter--;
        }  
        if(digitalRead(knobDT) != currentStateCLK)
        { 
            counter++;
        }
        value = counter / 2;
        
        if(value < 1)
        {
          counter = 1 * 2;
          value = counter / 2;
        }
        if(value >= 3)
        {
          counter = 3 * 2;
          value = counter / 2;
        }
  
      }
      //Serial.print("NOW ");  Serial.println(value);
      digitalWrite(STEP_LIN, HIGH);
      //delayMicroseconds(del);
      digitalWrite(STEP_LIN, LOW);
      previousStateCLK = currentStateCLK;
    }
  
  //REVERSE
    if(value == 3)
    {
      u8x8.clear();
      u8x8.drawString(0,0,"REVERSE");
    }
    while(value == 3)
    {
      linearDriver.shaft(false);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        if(digitalRead(knobDT) == currentStateCLK)
        {
            counter--;
        }  
        if(digitalRead(knobDT) != currentStateCLK)
        { 
            counter++;
        }
        value = counter / 2;
        
        if(value < 1)
        {
          counter = 1 * 2;
          value = counter / 2;
        }
        if(value >= 3)
        {
          counter = 3 * 2;
          value = counter / 2;
        }
      }
      //Serial.print("Value: ");  Serial.println(value);
      digitalWrite(STEP_LIN, HIGH);
      //delayMicroseconds(del);
      digitalWrite(STEP_LIN, LOW);
      previousStateCLK = currentStateCLK;
    }
    if (digitalRead(A0) == 0)
    { 
      posA = 0;
      Serial.print("POSITION A: "); Serial.println(posA);
      delay(500);
      selectBeta();
    }
  }
}
void selectBeta()
{
  Serial.println("SELECTING ANGLE BETA");
  u8x8.clear();
  u8x8.drawString(0,0,"SELECT BETA");
  while(1)
  {
     panDriver.microsteps (0);
      //Serial.println("waiting");
     currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }
      Serial.print("Value: ");  Serial.println(value);
    }
    previousStateCLK = currentStateCLK;

//CLOCKWISE    
  if(value == 1)
  {
    u8x8.clear();
    u8x8.drawString(0,0,"CLOCKWISE");
  }
  while(value == 1)
  {
    panDriver.shaft(true);
    currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }

    }
      //Serial.print("NOW ");  Serial.println(value);
      digitalWrite(STEP_PAN, HIGH);
      //delayMicroseconds(del);
      digitalWrite(STEP_PAN, LOW);
     // alpha++;
    previousStateCLK = currentStateCLK;
  }

//COUNTER CLOCKWISE
  if(value == 3)
  {
    u8x8.clear();
    u8x8.drawString(0,0,"COUNT CLOCKWISE");
  }
  while(value == 3)
  {
    panDriver.shaft(false);
    currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
 
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
          
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }

    }
    digitalWrite(STEP_PAN, HIGH);
    digitalWrite(STEP_PAN, LOW);
    //alpha--; 
    previousStateCLK = currentStateCLK;
  }
  if (digitalRead(A0) == 0)
  { 
      alpha = 0;
      Serial.print("ANGLE ALPHA A: "); Serial.println(alpha);
      delay(500);
      selectA();
    }
  }
}
void selectA()
{
  Serial.println("SELECTING POSITION A");
  u8x8.clear();
  u8x8.drawString(0,0,"SELECT POS A");
  while(1)
  {
    currentStateCLK = digitalRead(knobCLK);
     
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }
      //Serial.print("Value: ");  Serial.println(value);
    }
    previousStateCLK = currentStateCLK;
    
  //FORWARD
    if(value == 1)
    {
        u8x8.clear();
        u8x8.drawString(0,0,"FORWARD");
    }
    while(value == 1)
    {
      linearDriver.shaft(true);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        if(digitalRead(knobDT) == currentStateCLK)
        {
            counter--;
        }  
        if(digitalRead(knobDT) != currentStateCLK)
        { 
            counter++;
        }
        value = counter / 2;
        
        if(value < 1)
        {
          counter = 1 * 2;
          value = counter / 2;
        }
        if(value >= 3)
        {
          counter = 3 * 2;
          value = counter / 2;
        }
  
      }
      //Serial.print("NOW ");  Serial.println(value);
      digitalWrite(STEP_LIN, HIGH);
      //delayMicroseconds(del);
      digitalWrite(STEP_LIN, LOW);
      posA++;
      previousStateCLK = currentStateCLK;
    }
  
  //REVERSE
    if(value == 3)
    {
      u8x8.clear();
      u8x8.drawString(0,0,"REVERSE");
    }
    while(value == 3)
    {
      linearDriver.shaft(false);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        if(digitalRead(knobDT) == currentStateCLK)
        {
            counter--;
        }  
        if(digitalRead(knobDT) != currentStateCLK)
        { 
            counter++;
        }
        value = counter / 2;
        
        if(value < 1)
        {
          counter = 1 * 2;
          value = counter / 2;
        }
        if(value >= 3)
        {
          counter = 3 * 2;
          value = counter / 2;
        }
      }
      //Serial.print("Value: ");  Serial.println(value);
      digitalWrite(STEP_LIN, HIGH);
      //delayMicroseconds(del);
      digitalWrite(STEP_LIN, LOW);
      previousStateCLK = currentStateCLK;
      posA--;
    }
    if (digitalRead(A0) == 0)
    { 
      Serial.print("POSITION A: "); Serial.println(posA);
      delay(500);
      selectAlpha();
    }
  }

}
void selectAlpha()
{
  Serial.println("SELECTING ANGLE ALPHA");
  u8x8.clear();
  u8x8.drawString(0,0,"SELECT ALPHA");
  while(1)
  {
     panDriver.microsteps (0);
      //Serial.println("waiting");
     currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }
      Serial.print("Value: ");  Serial.println(value);
    }
    previousStateCLK = currentStateCLK;

//CLOCKWISE    
  if(value == 1)
  {
    u8x8.clear();
    u8x8.drawString(0,0,"CLOCKWISE");
  }
  while(value == 1)
  {
    panDriver.shaft(true);
    currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }

    }
      //Serial.print("NOW ");  Serial.println(value);
      digitalWrite(STEP_PAN, HIGH);
      //delayMicroseconds(del);
      digitalWrite(STEP_PAN, LOW);
      alpha++;
    previousStateCLK = currentStateCLK;
  }

//COUNTER CLOCKWISE
  if(value == 3)
  {
    u8x8.clear();
    u8x8.drawString(0,0,"COUNT CLOCKWISE");
  }
  while(value == 3)
  {
    panDriver.shaft(false);
    currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter--;
 
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter++;
          
      }
      value = counter / 2;
      
      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 3)
      {
        counter = 3 * 2;
        value = counter / 2;
      }

    }
    digitalWrite(STEP_PAN, HIGH);
    digitalWrite(STEP_PAN, LOW);
    alpha--; 
    previousStateCLK = currentStateCLK;
  }
  if (digitalRead(A0) == 0)
  { 
      Serial.print("ANGLE ALPHA: "); Serial.println(alpha);
      delay(500);
      posA = posA*128;
      alpha=alpha*256;
      //selectA();
    u8x8.clear();
    u8x8.drawString(0,0,"MOVING");
    moveMode();
    }
  }

}
void selectSpeed()
{

}











//  currentStateCLK = digitalRead(knobCLK);
//  
//  if (currentStateCLK != previousStateCLK)
//  {
//    if(digitalRead(knobDT) == currentStateCLK)
//    {
//        counter--;
//    }  
//    if(digitalRead(knobDT) != currentStateCLK)
//    { 
//        counter++;
//    }
//    value = counter / 2;
//    
//    if(value < 1)
//    {
//      counter = 1 * 2;
//      value = counter / 2;
//    }
//    if(value >= 4)
//    {
//      //Serial.println(3);
//      counter = 4 * 2;
//      value = counter / 2;
//    }
//
//    //value = counter * 10000;
//    Serial.print("Value: ");  Serial.println(value);
//    //stepsToShow = (counter - prevCounter)*10000;
//    //Serial.print(" StepsToShow: ");  Serial.println(stepsToShow);
////    prevCounter = counter;
////    //Serial.println(stepsToShow);
////    if( stepsToShow > 0)
////    {
////      linearDriver.shaft(true);
////      //Serial.println("Positive");
////    }
////    else
////    {
////      linearDriver.shaft(false);
////      //Serial.println("negative");
////    }
//
//    
///*///////////////////////////// Two options 0 and 1
//    if(value > 1)
//    {
//      counter = 1 * 2;
//      value = counter / 2;
//    }
//    else if (value < 0)
//    {
//      counter = 0 * 2;
//      value = counter / 2;
//    }
//*//////////////////////////////
//    //Serial.print("Direction: "); Serial.print(encdir); 
//    //Serial.print(" - - Value: "); Serial.println(value);
//  }
//      previousStateCLK = currentStateCLK;
//    if(value == 2)
//
//    if(value == 1)
//    {
//      linearDriver.shaft(true);
//      for(i = 0; i < 1000; i++)
//      {
//        //Serial.println(i);
//        delayMicroseconds(del);
//        digitalWrite(STEP_LIN, HIGH);
//        //delayMicroseconds(del);
//        digitalWrite(STEP_LIN, LOW);
//      }
//    }

//
//  if(posA > 0)
//  {
//    linearDriver.shaft(true);
//  }
//  else
//  {
//    linearDriver.shaft(false);
//  }
//
//  if(alpha > 0)
//  {
//    panDriver.shaft(true);
//  }
//  else
//  {
//    panDriver.shaft(false);
//  }
//
//  
//  multiplier = round(((float)abs(posA)) / ((float)6680));
//  Serial.println(multiplier);
//  //del = 300;
//  long i = 0;
////  del = 30;
//
//  
//  for(i = abs(posA); i >= 0; i--)
//  {
//    //Serial.println(i);
//    delayMicroseconds(del);
//    digitalWrite(STEP_LIN, HIGH);
//    //delayMicroseconds(del);
//    digitalWrite(STEP_LIN, LOW);
//    if((i % multiplier) == 0) // to increase pan resolution I could male the lin fire every 10th and that would dilute by 10
//    {
//       digitalWrite(STEP_PAN, HIGH);
//        //delayMicroseconds(del);
////       Serial.println("NOW");
////       Serial.println(i);
//       digitalWrite(STEP_PAN, LOW);
//    }
//  }
//  Serial.println(i);
//  //delay(5000);
//  Serial.println("DONE");


//

































  
//  for(i = 0; i < 1000000; i++)
//  {
//    //Serial.println(i);
//    delayMicroseconds(del);
//    digitalWrite(STEP_LIN, HIGH);
//    delayMicroseconds(del);
//    digitalWrite(STEP_LIN, LOW);
//  }
//  delay(2000);


//PROPER DELAY PHASE 2
//  for(i = 0; i < 1000000; i++)
//  {
//    if(millis() * 10 < timeX + del)
//    {
//      digitalWrite(STEP_LIN, HIGH);  
//    } 
//    else if(millis()* 10 < timeX + 2*del)
//    {
//      digitalWrite(STEP_LIN, LOW);
//    }
//    else
//    {
//      timeX = millis() * 10;
//    }
//  }

//PHASE 1
//  posB = 0;
//  posA = 1000000;
//
//    for(i = 0; i < posA; i++)
//    {
//      Serial.println(i);
//      digitalWrite(STEP_LIN, HIGH);
//      delayMicroseconds(del);
//      digitalWrite(STEP_LIN, LOW);
//    }
//    delay(2000);



//  while(1)
//  {
//    Serial.println("DONE");
//  }
  
//        del = analogRead(speedControlPIN);
//      del = map(del, 0, 1023, 6, 40);
//      Serial.println(del);
//  if (analogRead(A6) > 1000)
//  {
//    linearDriver.shaft(false);
//
// 
//    while (analogRead(A6) > 1000)
//    {
//      del = analogRead(speedControlPIN);
//      del = map(del, 0, 1023, 6, 40);
//
//      //Serial.println(del);

////      //digitalWrite(STEP_PAN, !digitalRead(STEP_PAN));
////      //digitalWrite(STEP_LIN, !digitalRead(STEP_LIN)); // Step
////  
////      digitalWrite(STEP_LIN, HIGH);
////      delayMicroseconds(100);
////      digitalWrite(STEP_LIN, LOW);
////      delayMicroseconds(100);
//    }
//  }
//  
//  if (analogRead(A7) > 1000)
//  {
//    linearDriver.shaft(true);    
//    while (analogRead(A7) > 1000)
//    {
//      
//      //digitalWrite(STEP_PAN, !digitalRead(STEP_PAN));
//      //digitalWrite(STEP_LIN, !digitalRead(STEP_LIN)); // Step
//  
//      digitalWrite(STEP_LIN, HIGH);
//      delayMicroseconds(100);
//      digitalWrite(STEP_LIN, LOW);
//      delayMicroseconds(100);  
//    }
//  }

    //Serial.println(x);

//  linearDriver.shaft(true);
//  for ( x = 0; x < 256000; x++) 
//  {
//    digitalWrite(STEP_LIN, !digitalRead(STEP_LIN)); // Step
//    delayMicroseconds(20);
//  }
 // linearDriver.shaft(false);

//    for ( x = 0; x < 2560; x++) {
//  digitalWrite(STEP_LIN, !digitalRead(STEP_LIN)); // Step
//  delayMicroseconds(10000);
//  }
//  for ( x = 0; x < 25600; x++) {
//  digitalWrite(STEP_LIN, !digitalRead(STEP_LIN)); // Step
//  delayMicroseconds(200);
//  }
//  linearDriver.shaft(true);
//  
//  //delay (1000);
