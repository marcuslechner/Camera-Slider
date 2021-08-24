//Marcus Lechner
//Help from mahmud
//                                                                                                                                                                                                                                                                                             
//efficient setup speeds
//less use of code. Plan on using setelect Beta and select posB twice with return of a long. dialog will be set in the regular mode and timelapse mode functoins
//why negfative multi

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

U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE); 


// Create driver that uses SoftwareSerial for communication
TMC2208Stepper linearDriver = TMC2208Stepper(RX_LIN, TX_LIN);
TMC2208Stepper panDriver = TMC2208Stepper(RX_PAN, TX_PAN);

int prevValue = 50;
int speedSelect = 0;
long del = 0;
long counter = 4;
int value = 2;
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
//long stepLin = 0;
//long stepPan = 0;
//long timingN = 0;
//long timingO = 0;
float hours = 0.00;

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

void setup() 
{  
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.clear();
  u8x8.drawString(0,0,"Starting up the");
  u8x8.drawString(0,2,"Lechner Slider");
  
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
  linearDriver.en_spreadCycle (false); //Spread Cycle enabled. offers smooth and quiet operation
  linearDriver.mstep_reg_select(true);  //Microsteps set by the registers

  //PAN STEPPER
  panDriver.pdn_disable(true);     // Use PDN/UART pin for communication
  panDriver.I_scale_analog(false); // Use internal voltage reference
  panDriver.rms_current(1100);      // Set panDriver current = 500mA, 0.5 multiplier for hold current and RSENSE = 0.11.
  panDriver.toff(2);               // Enable driver in software
  panDriver.intpol (true);
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
}

void loop() 
{
  selectMode();
}
void selectMode()
{
  u8x8.clear();
  u8x8.drawString(0,0,"Select a Mode:");
  counter = 0;
  value = 0;
  value = prevValue;
  prevValue = 0;
  while(1)
  {
    currentStateCLK = digitalRead(knobCLK);
     
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter++;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter--;
      }
      value = counter / 2;

      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 2)
      {
        counter = 4;
        value = counter / 2;
      }
    }
    
    previousStateCLK = currentStateCLK;

    if(value!= prevValue)
    {
      if(value == 1)
      {
        //hours = (float)value * 0.25;
        u8x8.clearLine(2);
        u8x8.clearLine(3);
        u8x8.setCursor(0,2);
        u8x8.print("1. Slide and Pan");
      }
      if(value == 2)
      {
        //hours = (float)value * 0.25;
        u8x8.clearLine(2);
        u8x8.clearLine(3);
        u8x8.setCursor(0,2);
        u8x8.print("2. Time Lapse");
      }
    }
    prevValue = value;
    if (digitalRead(A0) == 0)
    { 
      if(value == 1)
      {
        delay(500);
        regularMode();
      }
      if(value == 2)
      {
        delay(500);
        timeLapseMode();
      }
      return(0);
    }
  }
}
void regularMode()
{
  selectB();
  selectBeta();
  selectA();
  selectAlpha();
  selectSpeed();
  moveMode();
  selectMode();
}
void timeLapseMode()
{
  selectB();
  selectBeta();
  selectA();
  selectAlpha();
  selectTime();
  moveMode();
  selectMode();
}
void currentState()
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
void selectB()
{
  linearDriver.microsteps (0); 
  Serial.println("SELECTING POSITION B");
  u8x8.clear();
  u8x8.drawString(0,0,"Set End Pos:");
  value = 2;
  counter = 4;
  while(1)
  {
    currentStateCLK = digitalRead(knobCLK);
     
    if (currentStateCLK != previousStateCLK)
    {
      currentState();
    }
    previousStateCLK = currentStateCLK;
    
  //FORWARD
    if(value == 1)
    {
      u8x8.clearLine(2);
      u8x8.clearLine(3);
      u8x8.drawString(0,2,"Right");
    }
    while(value == 1)
    {
      linearDriver.shaft(true);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        currentState();
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
      u8x8.clearLine(2);
      u8x8.clearLine(3);
      u8x8.drawString(0,2,"Left");
    }
    while(value == 3)
    {
      linearDriver.shaft(false);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        currentState();
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
      //selectBeta();
      return(0);
    }
  }
}
void selectBeta()
{
  Serial.println("SELECTING ANGLE BETA");
  u8x8.clear();
  u8x8.drawString(0,0,"Set End Angle:");
  value = 2;
  counter = 4;
  
  while(1)
  {
     panDriver.microsteps (16);
      //Serial.println("waiting");
     currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      currentState();
    }
    previousStateCLK = currentStateCLK;

//CLOCKWISE    
  if(value == 1)
  {
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0,2,"Clockwise");
  }
  while(value == 1)
  {
    panDriver.shaft(false);
    currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      currentState();
    }

    digitalWrite(STEP_PAN, HIGH);
    digitalWrite(STEP_PAN, LOW);

    
    previousStateCLK = currentStateCLK;
  }

//COUNTER CLOCKWISE
  if(value == 3)
  {
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0,2,"Anti-Clockwise");
  }
  while(value == 3)
  {
    panDriver.shaft(true);
    currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      currentState();
    }
    
    digitalWrite(STEP_PAN, HIGH);
    digitalWrite(STEP_PAN, LOW);


    previousStateCLK = currentStateCLK;
  }
  if (digitalRead(A0) == 0)
  { 
      alpha = 0;
      Serial.print("ANGLE ALPHA A: "); Serial.println(alpha);
      delay(500);
      //selectA();
      return(0);
    }
  }
}
void selectA()
{
  linearDriver.microsteps (0); 
  Serial.println("SELECTING POSITION A");
  u8x8.clear();
  u8x8.drawString(0,0,"Set Start Pos:");
  value = 2;
  counter = 4;
  while(1)
  {
    currentStateCLK = digitalRead(knobCLK);
     
    if (currentStateCLK != previousStateCLK)
    {
      currentState();
    }
    previousStateCLK = currentStateCLK;
    
  //FORWARD
    if(value == 1)
    {
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0,2,"Right");
    }
    while(value == 1)
    {
      linearDriver.shaft(true);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        currentState();
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
      u8x8.clearLine(2);
      u8x8.clearLine(3);
      u8x8.drawString(0,2,"Left");
    }
    while(value == 3)
    {
      linearDriver.shaft(false);
      currentStateCLK = digitalRead(knobCLK);
      
      if (currentStateCLK != previousStateCLK)
      {
        currentState();
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
      //selectAlpha();
      return(0);
    }
  }

}
void selectAlpha()
{
  Serial.println("SELECTING ANGLE ALPHA");
  u8x8.clear();
  u8x8.drawString(0,0,"Set Start Angle:");
  value = 2;
  counter = 4;
  while(1)
  {
     panDriver.microsteps (16);
      //Serial.println("waiting");
     currentStateCLK = digitalRead(knobCLK);
    
    if (currentStateCLK != previousStateCLK)
    {
      currentState();
    }
    previousStateCLK = currentStateCLK;

//CLOCKWISE    
  if(value == 1)
  {
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0,2,"Clockwise");
  }
  while(value == 1)
  {
    panDriver.shaft(false);
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

//COUNTER CLOCKWISE
  if(value == 3)
  {
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0,2,"Anti-Clockwise");
  }
  while(value == 3)
  {
    panDriver.shaft(true);
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
  if (digitalRead(A0) == 0)
  { 
      Serial.print("ANGLE ALPHA: "); Serial.println(alpha);
      delay(500);
      posA = posA*128;
      alpha=alpha*16;
      //selectA();
    u8x8.clear();
    u8x8.drawString(0,0,"MOVING");
    //moveMode();/////////////////////////////////
    return(0);
    }
  }

}
void selectSpeed()
{
  //u8x8.setFont(u8x8_font_8x13_1x2_f); 
  Serial.println("speed select");
  u8x8.clear();
  u8x8.drawString(0,0,"Select Speed:");
  
  counter = 170;
  value = counter/2;
  prevValue = 0;
  while(1)
  {
    currentStateCLK = digitalRead(knobCLK);
     
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter++;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter--;
      }
      value = counter / 2;

      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 100)
      {
        counter = 100 * 2;
        value = counter / 2;
      }
      //Serial.print("Value: ");  Serial.println(value);
    }
    previousStateCLK = currentStateCLK;

    if(value!= prevValue)
    {
      u8x8.clearLine(2);
      u8x8.clearLine(3);
      u8x8.setCursor(0,2);
      u8x8.print(value);
    }
    prevValue = value;
    if (digitalRead(A0) == 0)
    { 
      delay(500);
      speedSelect = value;
      del = (100 - speedSelect)*40;
      return(0);
    }
  }
   speedSelect = value;
  //del = ((100 - speedSelect) * 2) + 5;
  del = 0;
  return(0);
}
void selectTime()
{
  //u8x8.setFont(u8x8_font_8x13_1x2_f); 
  Serial.println("speed time");
  u8x8.clear();
  u8x8.drawString(0,0,"Hours to Lapse:");
  counter = 0;
  value = 0;
  value = prevValue;
  prevValue = 0;
  while(1)
  {
    currentStateCLK = digitalRead(knobCLK);
     
    if (currentStateCLK != previousStateCLK)
    {
      if(digitalRead(knobDT) == currentStateCLK)
      {
          counter++;
      }  
      if(digitalRead(knobDT) != currentStateCLK)
      { 
          counter--;
      }
      value = counter / 2;

      if(value < 1)
      {
        counter = 1 * 2;
        value = counter / 2;
      }
      if(value >= 100)
      {
        counter = 100 * 2;
        value = counter / 2;
      }
      //Serial.print("Value: ");  Serial.println(value);
    }
    previousStateCLK = currentStateCLK;

    if(value!= prevValue)
    {
      hours = (float)value * 0.25;
      u8x8.clearLine(2);
      u8x8.clearLine(3);
      u8x8.setCursor(0,2);
      u8x8.print(hours);
    }
    prevValue = value;
    if (digitalRead(A0) == 0)
    { 
      delay(500);
      Serial.println(posA);
      del = round(0.9* hours * (float)3600000000)/((float)abs(posA));
      Serial.print("delay: "); Serial.println(del);
      Serial.print("overall delay "); Serial.println(del*abs(posA));
      return(0);
    }
  }
  return(0);
}
void moveMode()
{
  u8x8.clear();
  u8x8.drawString(0,0,"Calculating...");
  u8x8.clearLine(2);
  u8x8.clearLine(3);
  u8x8.drawString(0,2,"Please Wait...");
  
  multiplier1 = 0;
  multiplier2 = 0;
  multiplier3 = 0;
  multiplier4 = 0;
  multiplier5 = 0;
  
  linearDriver.microsteps (128);
  panDriver.microsteps (256);
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
    panDriver.shaft(true);
  }
  else
  {
    Serial.println(alpha);
    panDriver.shaft(false);
  }
  posA = abs(posA);
  alpha = abs(alpha);

  if (posA > alpha)
  {
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
    u8x8.drawString(0,0,"Now Executing");
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0,2,"Smooth Move");
  //  timingO = millis();
  //  Serial.print("timingO: "); Serial.println(timingO);
    for(i = 0; i <= posA; i++)
    {
  
      while((i%multiplier1 != 0)&&(i % multiplier2 != 0)&&(i % multiplier3 != 0)&&(i % multiplier4 != 0)&&(i % multiplier5 != 0)&&(i <= posA))
      {
        delayMicroseconds(del);
        digitalWrite(STEP_LIN, HIGH);
        digitalWrite(STEP_LIN, LOW);
        //stepLin++;
        i++;
      }
      if(i%multiplier1 == 0)
      {
        //Serial.println(1);
        digitalWrite(STEP_PAN, HIGH);
        digitalWrite(STEP_PAN, LOW);
        //stepPan++;
      }
      if(i%multiplier2 == 0)
      {
        //Serial.println(2);
        digitalWrite(STEP_PAN, HIGH);
        digitalWrite(STEP_PAN, LOW);
        //stepPan++;
      }
      if(i%multiplier3 == 0)
      {
        //Serial.println(3);
        digitalWrite(STEP_PAN, HIGH);
        digitalWrite(STEP_PAN, LOW);
        //stepPan++;
      }
      if(i%multiplier4 == 0)
      {
        //Serial.println(4);
        digitalWrite(STEP_PAN, HIGH);
        digitalWrite(STEP_PAN, LOW);
        //stepPan++;
      }
      if(i%multiplier5 == 0)
      {
        //Serial.println(5);
        digitalWrite(STEP_PAN, HIGH);
        digitalWrite(STEP_PAN, LOW);
        //stepPan++;
      }
  
      digitalWrite(STEP_LIN, HIGH);
      digitalWrite(STEP_LIN, LOW);
      //stepLin++;
      delayMicroseconds(del);
      
    }
    //Serial.println(i);
    Serial.println("DONE");
    //Serial.print("LINEAR STEPS NEEDED: "); Serial.print(posA);Serial.print("   LINEAR STEPS DONE: "); Serial.println(stepLin);
    //Serial.print("PAN STEPS NEEDED: "); Serial.print(alpha);Serial.print("   PAN STEPS DONE: "); Serial.println(stepPan);
    Serial.print("millis now: "); Serial.println(millis());
   // timingN = (millis() - timingO);
   // Serial.print("time elapsed: "); Serial.println(timingN);
    //while(1);
    //return(0);
    selectMode();
  }
  
  else
  {
    stepsLeft = posA;
    //Serial.print("Actual Multiplier: "); Serial.println((((float)abs(posA )) / ((float)alpha)));
  
    if(stepsLeft > 0)
    {
      multiplier1 = (1 + (alpha / posA));
      Serial.print("Multiplier1: ");Serial.println(multiplier1);
      //stepsLeft = posA;
      for(i = 0; i < alpha; i++)
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
      multiplier2 = (1 + (alpha / stepsLeft)); 
      Serial.print("Multiplier2: ");Serial.println(multiplier2);
      for(i = 0; i < alpha; i++)
      {
        if(i % multiplier2 == 0)
        {
          stepsDone++;
        }
      }
      stepsLeft = posA - stepsDone;
      Serial.print("STEPS DONE3: "); Serial.println(stepsDone); 
      Serial.print("STEPS LEFT3: "); Serial.println(stepsLeft);
    }
    
    if(stepsLeft > 0)
    {
      multiplier3 = (1 + (alpha / stepsLeft)); 
      Serial.print("Multiplier3: ");Serial.println(multiplier3);
      for(i = 0; i < alpha; i++)
      {
        if(i % multiplier3 == 0)
        {
          stepsDone++;
        }
      }
      stepsLeft = posA - stepsDone;
      Serial.print("STEPS DONE4: "); Serial.println(stepsDone); 
      Serial.print("STEPS LEFT4: "); Serial.println(stepsLeft);
    }
  
    if(stepsLeft > 0)
    {
      multiplier4 = (1 + (alpha / stepsLeft)); 
      Serial.print("Multiplier4: ");Serial.println(multiplier4);
      for(i = 0; i < alpha; i++)
      {
        if(i % multiplier4 == 0)
        {
          stepsDone++;
        }
      }
      stepsLeft = posA - stepsDone;
      Serial.print("STEPS DONE5: "); Serial.println(stepsDone); 
      Serial.print("STEPS LEFT5: "); Serial.println(stepsLeft);
    }
  
    if(stepsLeft > 0)
    {
      multiplier5 = (1 + (alpha / stepsLeft)); 
      Serial.print("Multiplier5: ");Serial.println(multiplier5);
      for(i = 0; i < alpha; i++)
      {
        if(i % multiplier5 == 0)
        {
          stepsDone++;
        }
      }
      stepsLeft = posA - stepsDone;
      Serial.print("STEPS DONE2: "); Serial.println(stepsDone); 
      Serial.print("STEPS LEFT2: "); Serial.println(stepsLeft);
    }
    //int multiplier3 = (1 + (posA/(alpha - ((alpha/multiplier2)*(((float)abs(posA )) / ((float)alpha))))));
    //Serial.print("Multiplier3: ");Serial.println(multiplier3);
      
      
    u8x8.clear();
    u8x8.drawString(0,0,"Now Executing");
    u8x8.clearLine(2);
    u8x8.clearLine(3);
    u8x8.drawString(0,2,"Smooth Move");
  //  timingO = millis();
  //  Serial.print("timingO: "); Serial.println(timingO);
    for(i = 0; i <= alpha; i++)
    {
  
      while((i%multiplier1 != 0)&&(i % multiplier2 != 0)&&(i % multiplier3 != 0)&&(i % multiplier4 != 0)&&(i % multiplier5 != 0)&&(i <= alpha))
      {
        delayMicroseconds(del);
        digitalWrite(STEP_PAN, HIGH);
        digitalWrite(STEP_PAN, LOW);
        //stepLin++;
        i++;
      }
      if(i%multiplier1 == 0)
      {
        //Serial.println(1);
        digitalWrite(STEP_LIN, HIGH);
        digitalWrite(STEP_LIN, LOW);
        //stepPan++;
      }
      if(i%multiplier2 == 0)
      {
        //Serial.println(2);
        digitalWrite(STEP_LIN, HIGH);
        digitalWrite(STEP_LIN, LOW);
        //stepPan++;
      }
      if(i%multiplier3 == 0)
      {
        //Serial.println(3);
        digitalWrite(STEP_LIN, HIGH);
        digitalWrite(STEP_LIN, LOW);
        //stepPan++;
      }
      if(i%multiplier4 == 0)
      {
        //Serial.println(4);
        digitalWrite(STEP_LIN, HIGH);
        digitalWrite(STEP_LIN, LOW);
        //stepPan++;
      }
      if(i%multiplier5 == 0)
      {
        //Serial.println(5);
        digitalWrite(STEP_LIN, HIGH);
        digitalWrite(STEP_LIN, LOW);
        //stepPan++;
      }
  
      digitalWrite(STEP_PAN, HIGH);
      digitalWrite(STEP_PAN, LOW);
//      stepLin++;
      delayMicroseconds(del);
      
    }
    //Serial.println(i);
//      Serial.println("DONE");
//      Serial.print("LINEAR STEPS NEEDED: "); Serial.print(posA);Serial.print("   LINEAR STEPS DONE: "); Serial.println(stepLin);
//      Serial.print("PAN STEPS NEEDED: "); Serial.print(alpha);Serial.print("   PAN STEPS DONE: "); Serial.println(stepPan);
//      Serial.print("millis now: "); Serial.println(millis());
   // timingN = (millis() - timingO);
   // Serial.print("time elapsed: "); Serial.println(timingN);
    //while(1);
    //return(0);
    selectMode();
  }
}
