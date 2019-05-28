//Code von Lennart O.
//Translated and adapted to Arduino Leonardo CAN-BUS by Daniel Montero, 
//Libraries to integrate
// CAN ID to send messages = 0x1806E7F4 if protocol 998, 0x1806E5F4 if protocol 1430
// CAN ID to receive messages = 0x18FF50E7 if protocol 998, 0x18FF50E5 if protocol 1430

#include <LiquidCrystal.h>
#include "LcdKeypad.h"
#include "MenuData.h"
#include <TimerOne.h>

#define BUTTON 1

float button = 0;
int charger1 = 0; // Values charger1 and charger2 are used to count how many chargers are sending data
int charger2 = 0;
int manualchargercount = 1; // Value used to manually tell how many chargers are connected in case all share the same CAN address.
int NumberofChargers = charger1 + charger2; // NumberofChargers is used to correct total charging current and kWh charged in the menu
long statusCAN = 0 ; // Value to tell the LCD if CAN connection is OK. 0 = not OK, 1 = OK.
long startCAN = 0; // Value to start CAN comm using the "Start CAN-BUS" menu. 0 = off; 1 = on.
long startTime = 0; // Value to start Time chronometer using the "Start CAN-BUS" menu. 0 = off; 1 = on.
int hours = 0;
int minutes = 0;
int seconds = 0;
int secondsdisplay = 0;
int tenths = 0;
long milliseconds = 0;
long chargecurrent = 320;
float chargepower = chargecurrent*0.103*max(NumberofChargers,1); // We assume the nominal voltage, 103 V to be the reference for power calculations.
long chargedraw = chargecurrent/2; // We calculate the Amps drawn to the AC source by assuming 220 V. Accounting from the charger losses, it is roughly half the DC charging amps.
int targetSOC = 100 ;
float chargedenergy = 0; // Value used to know the amount of charge put into the battery in realtime.
float maxvoltage = 1162;

#include <mcp_can.h> 
#include <SPI.h>
#include <SimpleTimer.h>

//Constants and variable definitions
#define LED_BUILTIN 23
#define SPI_CS_PIN 17 //CS Pin
#define POTI 1 //A1 for potentiometer
#define GRID 2 //A2 for the external switch that changes the current (and charging power) presets between low power (1300 W for 110 V outlets and 3300 W for 220 V and L2 charging)

word outputvoltage = 1162; //set max voltage to 116,2V (offset = 0,1)
word outputcurrent = chargecurrent; //set max current to 320 (32A, offset = 0,1)
float poti = 0;
int grid = 0;

unsigned long int sendId1 = 0x1806E7F4;
unsigned long int sendId2 = 0x1806E5F4;
unsigned char voltamp[8] = {highByte(outputvoltage), lowByte(outputvoltage), highByte(outputcurrent), lowByte(outputcurrent), 0x00,0x00,0x00,0x00};

unsigned char len = 0; //Length of received CAN message of either charger
unsigned char buf[8]; //Buffer for data from CAN message of either charger
unsigned char buf1[8]; //Buffer for data from CAN message of charger 1
unsigned char buf2[8]; //Buffer for data from CAN message of charger 2
unsigned long int receiveId; //ID of either charger
unsigned long int receiveId1; //ID of charger 1
unsigned long int receiveId2; //ID of charger 2
float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset

//Declare objects

MCP_CAN CAN(SPI_CS_PIN); //Set CS pin for SPI

SimpleTimer timer1; //Timer object generation

//Features

/************************************************
** Function name:           canRead1
** Descriptions:            read CAN message of charger 1
*************************************************/

void canRead1(){

  if(CAN_MSGAVAIL == CAN.checkReceive()){ //Check for messages

    CAN.readMsgBuf(&len, buf); // read data, len: data length, buf: data buffer

    receiveId = CAN.getCanId(); //Reading CAN-ID from either charger
    
    if(receiveId == 0x18FF50E7){ //CAN Bus ID from TC charger protocol 998
      receiveId1=receiveId;
       for(int i = 0; i<len; i++){ //Output data
      buf1[i]=buf[i];
       }
      charger1=1;      
      Serial.println("TC CAN Data received!");
      statusCAN=1;
      Serial.print("CAN ID: ");
      Serial.print(receiveId1, HEX); //Output ID

      Serial.print(" / CAN Data: ");
      for(int i = 0; i<len; i++){ //Output data

        if( buf1[i] < 0x10){ // Displaying zero if only one digit
          Serial.print("0");
        }

        Serial.print(buf1[i],HEX);
        Serial.print(" ");          // Spaces

      }

      Serial.println(); //Prints an empty paragraph

      Serial.print("Charging voltage: ");
      float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
      
      Serial.print(pv_voltage1);
      Serial.print(" V / Charging current: ");
      float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
      Serial.print(pv_current1);
  
      Serial.println(" A"); //Paragraph

      switch (buf1[4]) { //Read out error byte

        case B00000001: Serial.println("Error: hardware error");statusCAN=0;break;
        case B00000010: Serial.println("Error: overheating");statusCAN=0;break;
        case B00000100: Serial.println("Error: input voltage not allowed");statusCAN=0;break;
        case B00001000: Serial.println("Error: battery not connected");statusCAN=0;break;
        case B00010000: Serial.println("Error: CAN bus error");statusCAN=0;break;
        case B00001100: Serial.println("Error: No input voltage");statusCAN=0;break;

      }

    }
    else if(receiveId == 0x18FF50E5){ //CAN Bus ID from TC charger protocol 1430
      receiveId2=receiveId;
       for(int i = 0; i<len; i++){ //Output data
      buf2[i]=buf[i];
       }
      charger2=1;      
      Serial.println("TC CAN Data received!");
      statusCAN=1;
      Serial.print("CAN ID: ");
      Serial.print(receiveId2, HEX); //Output ID

      Serial.print(" / CAN Data: ");
      for(int i = 0; i<len; i++){ //Output data

        if( buf2[i] < 0x10){ // Displaying zero if only one digit
          Serial.print("0");
        }

        Serial.print(buf2[i],HEX);
        Serial.print(" ");          // Spaces

      }

      Serial.println(); //Prints an empty paragraph

      Serial.print("Charging voltage: ");
      float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
      Serial.print(pv_voltage2);
      Serial.print(" V / Charging current: ");
      float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
      Serial.print(pv_current2);
  
      Serial.println(" A"); //Paragraph

      switch (buf2[4]) { //Read out error byte

        case B00000001: Serial.println("Error: hardware error");statusCAN=0;break;
        case B00000010: Serial.println("Error: overheating");statusCAN=0;break;
        case B00000100: Serial.println("Error: input voltage not allowed");statusCAN=0;break;
        case B00001000: Serial.println("Error: battery not connected");statusCAN=0;break;
        case B00010000: Serial.println("Error: CAN bus error");statusCAN=0;break;
        case B00001100: Serial.println("Error: No input voltage");statusCAN=0;break;

      }

    }
  }

}
/************************************************
** Function name:           canRead2
** Descriptions:            read CAN message of charger 2
*************************************************/

void canRead2(){

  if(CAN_MSGAVAIL == CAN.checkReceive()){ //Check for messages

    CAN.readMsgBuf(&len, buf); // read data, len: data length, buf: data buffer

    receiveId = CAN.getCanId(); //Reading CAN-ID from either charger
    
    
    if(receiveId == 0x18FF50E5){ //CAN Bus ID from TC charger protocol 1430
      receiveId2=receiveId;
       for(int i = 0; i<len; i++){ //Output data
      buf2[i]=buf[i];
       }
      charger2=1;      
      Serial.println("TC CAN Data received!");
      statusCAN=1;
      Serial.print("CAN ID: ");
      Serial.print(receiveId2, HEX); //Output ID

      Serial.print(" / CAN Data: ");
      for(int i = 0; i<len; i++){ //Output data

        if( buf2[i] < 0x10){ // Displaying zero if only one digit
          Serial.print("0");
        }

        Serial.print(buf2[i],HEX);
        Serial.print(" ");          // Spaces

      }

      Serial.println(); //Prints an empty paragraph

      Serial.print("Charging voltage: ");
      float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
      Serial.print(pv_voltage2);
      Serial.print(" V / Charging current: ");
      float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
      Serial.print(pv_current2);
  
      Serial.println(" A"); //Paragraph

      switch (buf2[4]) { //Read out error byte

        case B00000001: Serial.println("Error: hardware error");statusCAN=0;break;
        case B00000010: Serial.println("Error: overheating");statusCAN=0;break;
        case B00000100: Serial.println("Error: input voltage not allowed");statusCAN=0;break;
        case B00001000: Serial.println("Error: battery not connected");statusCAN=0;break;
        case B00010000: Serial.println("Error: CAN bus error");statusCAN=0;break;
        case B00001100: Serial.println("Error: No input voltage");statusCAN=0;break;

      }

    }
  }

}
/************************************************
** Function name:           readGrid
** Descriptions:            read grid switch to adjust current
*************************************************/

void readGrid() {

  grid = analogRead(GRID); //Read from A2 input
  if (grid <= 500) {
    chargecurrent = 320; 
    outputcurrent = chargecurrent;
  }
  else {
    chargecurrent = 112;
    outputcurrent = chargecurrent;
  }
}

/************************************************
** Function name:           canWrite
** Descriptions:            write CAN message
*************************************************/
unsigned char data1[8];
unsigned char data2[8];
unsigned long int id1;
unsigned long int id2;
String canWrite1(unsigned char data1[8], unsigned long int id1){

  byte sndStat = CAN.sendMsgBuf(id1, 1, 8, data1); //Send message (ID, extended frame, data length, data)

  if(sndStat == CAN_OK) {//Status byte for transmission
    return "CAN message sent successfully to charger 1";
    statusCAN = 1;
  }
  else {
    return "Error during message transmission to charger 1";
    statusCAN = 0;
}
}
String canWrite2(unsigned char data2[8], unsigned long int id2){

  byte sndStat = CAN.sendMsgBuf(id2, 1, 8, data2); //Send message (ID, extended frame, data length, data)

  if(sndStat == CAN_OK) {//Status byte for transmission
    return "CAN message sent successfully to charger 2";
    statusCAN = 1;
  }
  else {
    return "Error during message transmission to charger 2";
    statusCAN = 0;
}
}

/************************************************
** Function name:           setVoltage
** Descriptions:            set target voltage
*************************************************/

void setVoltage(int t_voltage) { //can be used to set desired voltage to i.e. 80% SOC

  if(t_voltage >= 980 && t_voltage <= 1164){
    
    outputvoltage = t_voltage;
    
  }

 }

/************************************************
** Function name:           setCurrent
** Descriptions:            set target current
*************************************************/

void setCurrent(int t_current) { //can be used to reduce or adjust charging speed

  if(t_current >= 0 && t_current <= 320){
    
    outputcurrent = t_current;
    
  }

 }

/************************************************
** Function name:           myTimer1
** Descriptions:            function of timer1
*************************************************/

void myTimer1() { //Cyclic function called by the timer

  Serial.print("Set individual charging current: ");
  Serial.print((float)outputcurrent/10.0); //Output current setpoint
  Serial.println(" A");
  Serial.print("Total charging current: ");
  Serial.print((float)outputcurrent*NumberofChargers/10.0); //Output current setpoint
  Serial.println(" A");
  Serial.print("Set max SOC: ");
  Serial.print(targetSOC);
  Serial.print(" % (Max voltage ");
  
  Serial.print((float)outputvoltage/10.0); //Target SOC to stop charge
  Serial.println(" V)");
  
  unsigned char voltamp1[8] = {highByte(outputvoltage), lowByte(outputvoltage), highByte(outputcurrent), lowByte(outputcurrent), 0x00,0x00,0x00,0x00}; //Regenerate the message
  Serial.println(canWrite1(voltamp1, sendId1)); //Send message and output results
  unsigned char voltamp2[8] = {highByte(outputvoltage), lowByte(outputvoltage), highByte(outputcurrent), lowByte(outputcurrent), 0x00,0x00,0x00,0x00}; //Regenerate the message
  Serial.println(canWrite2(voltamp2, sendId2)); //Send message and output results
  canRead1(); //Call read function of charger 1
  canRead2(); //Call read function of charger 2
  
  Serial.println(); //Print a blank line

}


enum AppModeValues
{
  APP_NORMAL_MODE,
  APP_MENU_MODE,
  APP_TIMER_RUNNING,
  APP_PROCESS_MENU_CMD
};

byte appMode = APP_NORMAL_MODE; //APP_NORMAL_MODE

MenuManager Menu1(sampleMenu_Root, menuCount(sampleMenu_Root));

char strbuf[LCD_COLS + 1]; // one line of lcd display
byte btn;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 12);

void refreshMenuDisplay (byte refreshMode);
byte getNavAction();


void setup()
{
  Serial.begin(9600);
  delay(1000);
  Serial.println("Welcome to TCCharger Setup");
  backLightOn();
   pinMode(LED_BUILTIN, OUTPUT); //LED start
   digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  
  // set up the LCD's number of columns and rows:
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.setCursor(0,0);
  lcd.print("Welcome to");
  lcd.setCursor(0,1);
  lcd.print("Charger Setup");
  delay(2000);
  lcd.clear();
  readGrid();
  
      while(CAN_OK != CAN.begin(CAN_250KBPS)){ // CAN Bus initialisation
      Serial.println("CAN Initialization failed, restart");
      delay(200);
      }
      Serial.println("CAN initialization successful");
      timer1.setInterval(950, myTimer1); //Defining the time and function of the timer

  // fall in to timer mode by default.
  appMode = APP_MENU_MODE;

  refreshMenuDisplay(REFRESH_DESCEND);

  // Use soft PWM for backlight, as hardware PWM must be avoided for some LCD shields.
  Timer1.initialize();
  Timer1.attachInterrupt(lcdBacklightISR, 500);
  setBacklightBrightness(1);

}


void loop()                                                     // ----------------------------LOOP------------------------------//
{ 
   if (startCAN == 1) {
    timer1.run();
   appMode=APP_TIMER_RUNNING;
   float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
   float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
   float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
   float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
   float pv_current = pv_current1 + pv_current2 ;
  if (pv_current == 0 && seconds >=1) { // change to 0 after debugging!!!
    startTime=0;
    startCAN = 0;  
   }
   else {
    /*
   if (pv_current == 0) {
    startTime=0;
   }
   else {
   */
   if (startTime == 1) { 
     
    milliseconds = millis();
    appMode = APP_TIMER_RUNNING;
    if(milliseconds % 100 == 0){ //Only enter if it has passed tenth of a second
      tenths++;
      if (tenths == 10){
        tenths = 0;
        seconds++;
        secondsdisplay++;
        float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
        float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
        float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
        float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
        float pv_current = pv_current1 + pv_current2*manualchargercount ;
        chargedenergy = chargedenergy + pv_current1*pv_voltage1/1000/3600+pv_current2*manualchargercount*pv_voltage2/1000/3600;
        // Print real charging current as provided by the charger
        if (secondsdisplay <= 5) {
         lcd.setCursor(9,0);
         lcd.print("To ");
         if (targetSOC >= 100) {
           targetSOC = 100;
           lcd.print(targetSOC);
            lcd.print("%");
          }
          else {
           lcd.setCursor(12,0);
           lcd.print(" ");
           lcd.print(targetSOC);
           lcd.print("%");
          }
        float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
        float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
        float pv_current = pv_current1 + pv_current2*manualchargercount ;
        if (pv_current >= 10) {   
          float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
          float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
          float pv_current = pv_current1 + pv_current2*manualchargercount ;  
          lcd.setCursor(10,1);
          lcd.print(" ");
          lcd.print(pv_current,1);
          if (pv_current >= 100) {   
          float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
          float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
          float pv_current = pv_current1 + pv_current2*manualchargercount ;  
          lcd.setCursor(10,1);
          lcd.print(pv_current,1);
         }
         }
        else {
         lcd.setCursor(10,1);
         lcd.print("  ");
         float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
         float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
         float pv_current = pv_current1 + pv_current2*manualchargercount ;  
         lcd.print(pv_current,1);
        }
        lcd.print("A");  
     }
     else {
      if (targetSOC == 30) {
       outputvoltage = 999;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 40) {
       outputvoltage = 1009;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 50) {
       outputvoltage = 1025;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 60) {
       outputvoltage = 1046;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 70) {
       outputvoltage = 1070;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 80) {
       outputvoltage = 1097;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 90) {
       outputvoltage = 1126;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 95) {
       outputvoltage = 1136;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 100) {
       outputvoltage = 1162;
       maxvoltage = outputvoltage;
      }
      lcd.setCursor(9,0);
      strbuf[0] = 0b01111110; // forward arrow representing input prompt.
      strbuf[1] = 0;
      lcd.print(strbuf);
      float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
      float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
      float pv_voltagemax = max(pv_voltage1,pv_voltage2);
      float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);
      if (maxvoltage/10 >= 100) {          
       lcd.print(maxvoltage/10,1);
       lcd.print("V");
      }
      else {        
        lcd.setCursor(10,0);
        lcd.print(" ");
        lcd.print(maxvoltage/10,1);
        lcd.print("V");
      }
      if (pv_voltage >= 100) {
      lcd.setCursor(10,1);
      float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
      float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
      float pv_voltagemax = max(pv_voltage1,pv_voltage2);
      float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);
      lcd.print(pv_voltage,1);
      lcd.print("V");
      }
      else {
        if (pv_voltage < 100) {
          lcd.setCursor(10,1);
          lcd.print(" ");
          float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
          float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
          float pv_voltagemax = max(pv_voltage1,pv_voltage2);
          float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);      
          lcd.print(pv_voltage,1);
          lcd.print("V");
          if (pv_voltage < 10) {
          lcd.setCursor(10,1);
          lcd.print("  ");
          float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
          float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
          float pv_voltagemax = max(pv_voltage1,pv_voltage2);
          float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);      
          lcd.print(pv_voltage,1);
          lcd.print("V");
        }
        }
        else {
         lcd.setCursor(10,1);
         lcd.print(" ");
         float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
         float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
         float pv_voltagemax = max(pv_voltage1,pv_voltage2);
         float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);  
         lcd.print("V");
      }
      }
     }
      
         if (secondsdisplay >= 10) {
          secondsdisplay = 0;     
         }
         //Print kWh
    lcd.setCursor(0,1);
    if (chargedenergy < 10) {
      lcd.print(" ");
      lcd.setCursor(1,1);
      lcd.print(chargedenergy,1);
    }
    else {
      lcd.print(chargedenergy,1);
    }
    lcd.setCursor(4,1);
    lcd.print(" kWh @");
      
    
    
      }
    if(seconds == 60){ //When it has passed 60 seconds it count one minute
      seconds = 0;
      minutes++;
    }
    if(minutes == 60){ //When it has passed 60 minutes it count one hour
      minutes = 0;
      hours++;
    }
   //Print Time
    lcd.setCursor(0,0);
    if(hours < 10){
      lcd.print("0");
    }
    lcd.print(hours);
    lcd.print(":");
    lcd.setCursor(3,0);
    if(minutes < 10){
      lcd.print("0");
    }
    lcd.print(minutes);
    lcd.print(":");
    lcd.setCursor(6,0);
    if(seconds < 10){
      lcd.print("0");
    }
    lcd.print(seconds);
    
   }
  if (startTime == 0) {
    appMode = APP_MENU_MODE;
    milliseconds = 0 ;
    lcd.clear();
  }
   }
   }
   }
   btn = getButton();

  
  switch (appMode)
  {
    case APP_NORMAL_MODE :
      if (btn == BUTTON_UP_LONG_PRESSED)
      {
        appMode = APP_MENU_MODE;
        refreshMenuDisplay(REFRESH_DESCEND);
      }
      break;
    case APP_MENU_MODE :
    {
      byte menuMode = Menu1.handleNavigation(getNavAction, refreshMenuDisplay);

      if (menuMode == MENU_EXIT)
      {
        lcd.clear();
        lcd.print("Hold UP for menu");
        appMode = APP_NORMAL_MODE;
      }
      else if (menuMode == MENU_INVOKE_ITEM)
      {
        appMode = APP_PROCESS_MENU_CMD;
        

        // Indicate selected item.
        if (Menu1.getCurrentItemCmdId())
        {
          lcd.setCursor(0, 1);
          strbuf[0] = 0b01111110; // forward arrow representing input prompt.
          strbuf[1] = 0;
          lcd.print(strbuf);
        }
      }
      break;
    }
    case APP_PROCESS_MENU_CMD :
    {
      byte processingComplete = processMenuCommand(Menu1.getCurrentItemCmdId());

      if (processingComplete)
      {
        appMode = APP_MENU_MODE;
        // clear forward arrow
        lcd.setCursor(0, 1);
        strbuf[0] = ' '; // clear forward arrow
        strbuf[1] = 0;
        lcd.print(strbuf);
      }
    
      break;
    }
      case APP_TIMER_RUNNING : // These lines allow to stop time and CAN comm, so we can go to menu mode and change charge settings
      {
        if (btn == BUTTON_SELECT_LONG_PRESSED && statusCAN ==1) {
          startTime = 0;
          startCAN = 0;
          btn = 0;
          milliseconds = 0;
          startTime =0;
          lcd.setCursor(0,1);
          btn = getButton();
          lcd.print("Paused by user  ");
          appMode = APP_MENU_MODE;
        
          /*if (btn == BUTTON_SELECT_PRESSED && startTime==0) {
             appMode = APP_TIMER_RUNNING;
            startCAN=1;
            startTime=1;
           milliseconds = millis();
            break;
          } else {}*/
          }
      
        
        
        else {
          startTime = 1 ;
          milliseconds = millis();
        }
        break;
      }
    }
  }


//----------------------------------------------------------------------
// Addition or removal of menu items in MenuData.h will require this method
// to be modified accordingly. 
byte processMenuCommand(byte cmdId)
{
  byte complete = false;  // set to true when menu command processing complete.

  if (btn == BUTTON_SELECT_PRESSED)
  {
    complete = true;
  }

  switch (cmdId)
    {
    // TODO Process menu commands here:
    case mnuCmdCurrentSub :
    {
        // Print charging menu
        lcd.setCursor(12,0);
        if (chargedraw < 100) {
          lcd.setCursor(12,0);
          lcd.print (" ");
          lcd.print (chargedraw/10);
        }
          else {
          lcd.setCursor(12,0);
          lcd.print(chargedraw/10);
        }
        lcd.print(" A");
        lcd.setCursor(2,1);
        lcd.print("DC");
        if (chargecurrent < 100) {
          lcd.setCursor(5,1);
          lcd.print (" ");
          lcd.print (chargecurrent/10);
        }
          else {
          lcd.setCursor(5,1);
          lcd.print(chargecurrent/10);
        }
        lcd.setCursor(7,1);
        lcd.print("A");
        lcd.setCursor(10,1);
        if (chargepower/10 < 10) {
          lcd.setCursor(9,1);
          lcd.print (" ");
          lcd.print (chargepower/10,1);
         // Serial.print(chargepower);
        }
          else {
          lcd.setCursor(9,1);
          lcd.print(chargepower/10,1);
         // Serial.print(chargepower);
        }
        lcd.setCursor(13,1);
        lcd.print(" kW");
        if (btn == BUTTON_UP_PRESSED || btn == BUTTON_UP_LONG_PRESSED) {
          if (chargecurrent >= 320) {
            chargecurrent = 320 ;
            chargepower = chargecurrent * 0.103 ;
            chargedraw = chargecurrent / 2 ;
            outputcurrent = chargecurrent ;
            startCAN = 0 ;
            Serial.print(chargecurrent/10);
            Serial.print(" A DC current per charger");
            Serial.println();
          }
            else {
            chargecurrent = chargecurrent + 20;
            chargepower = chargecurrent * 0.103 ;
            chargedraw = chargecurrent / 2 ;
            outputcurrent = chargecurrent ;
            startCAN = 0 ;
            Serial.print(chargecurrent/10);
            Serial.print(" A DC current per charger");
            Serial.println();
            delay(100);
            }
          }
        if (btn == BUTTON_DOWN_PRESSED || btn == BUTTON_DOWN_LONG_PRESSED) {
          if (chargecurrent <= 20) {
            chargecurrent = 20 ;
            chargepower = chargecurrent * 0.103 ;
            chargedraw = chargecurrent / 2 ;
            outputcurrent = chargecurrent ;
            startCAN = 0 ;
            Serial.print(chargecurrent/10);
            Serial.print(" A DC current per charger");
            Serial.println();
            delay(100);
          }
            else {
            chargecurrent = chargecurrent - 20;
            chargepower = chargecurrent * 0.103 ;
            chargedraw = chargecurrent / 2 ;
            outputcurrent = chargecurrent ;
            startCAN = 0 ;
            Serial.print(chargecurrent/10);
            Serial.print(" A DC current per charger");
            Serial.println();
                        }
          }

     }
     break;
     case mnuCmdSOCSub :
     {
      if (targetSOC >= 100) {
        targetSOC = 100;
        lcd.setCursor(12,0);        
        lcd.print(targetSOC);
        lcd.print("%");
      }
      else {
        lcd.setCursor(12,0);
        lcd.print(" ");
        lcd.print(targetSOC);
        lcd.print("%");
      }
      if (btn == BUTTON_UP_PRESSED || btn == BUTTON_UP_LONG_PRESSED) {
        if (targetSOC >= 100) {
          targetSOC = 100;
        }
        else if(targetSOC < 90) {
          targetSOC = targetSOC + 10;
        }
        else if (targetSOC == 90) {
          targetSOC = targetSOC + 5;
        }
        else if (targetSOC == 95) {
          targetSOC = targetSOC + 5;
        }
      }
      if (btn == BUTTON_DOWN_PRESSED || btn == BUTTON_DOWN_LONG_PRESSED) {
        if (targetSOC <= 30) {
          targetSOC = 30;
        }
        else if (targetSOC <= 90) {
          targetSOC = targetSOC - 10;
        }
        else if (targetSOC == 100) {
          targetSOC = targetSOC - 5;
        }
        else if (targetSOC == 95) {
          targetSOC = targetSOC - 5;
        }
      }
      if (targetSOC == 30) {
        outputvoltage = 999;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 40) {
        outputvoltage = 1009;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 50) {
        outputvoltage = 1025;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 60) {
        outputvoltage = 1046;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 70) {
        outputvoltage = 1070;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 80) {
        outputvoltage = 1097;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 90) {
        outputvoltage = 1126;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 95) {
        outputvoltage = 1136;
        maxvoltage = outputvoltage;
      }
      if (targetSOC == 100) {
        outputvoltage = 1162;
        maxvoltage = outputvoltage;
      }
      if (maxvoltage/10 >= 100) {
      lcd.setCursor(2,1);
      lcd.print("V Max   ");
      lcd.print(maxvoltage/10,1);
      lcd.print("V");
      }
      else {
      lcd.setCursor(2,1);
      lcd.print("V Max    ");
      lcd.print(maxvoltage/10,1);
      lcd.print("V");     
      }
      }
     break;
          case mnuCmdNumberSub :
    {      
      lcd.setCursor(0,1);
      lcd.print("Min: 1 Max:4");
      lcd.setCursor(14,1);
      strbuf[0] = 0b01111110; // forward arrow representing input prompt.
      strbuf[1] = 0;
      lcd.print(strbuf);
      lcd.print(manualchargercount);
      if (btn == BUTTON_UP_PRESSED || btn == BUTTON_UP_LONG_PRESSED) {
        if (manualchargercount >= 4) {
          manualchargercount = 4;
        }
        else {
          manualchargercount++;
          lcd.setCursor(15,1);
          lcd.print(manualchargercount);          
        }
      }
      if (btn == BUTTON_DOWN_PRESSED || btn == BUTTON_DOWN_LONG_PRESSED) {
        if (manualchargercount <= 1) {
          manualchargercount = 1;
        }
        else {
          manualchargercount--;
          lcd.setCursor(15,1);
          lcd.print(manualchargercount);          
        }
      }
      
    }
    break;
     case mnuCmdCancomm :
       if (btn == BUTTON_SELECT_PRESSED || btn == BUTTON_SELECT_LONG_PRESSED) {
          lcd.setCursor(2,1);
          startTime = 1 ; 
          startCAN=1;
          timer1.run();        
          if(statusCAN == 1) { // IMPORTANT! CHANGE TO 1 AFTER FINISH DEBUGGING!!!                <---------- IMPORTANT HERE!
            lcd.print("CAN connected");
            delay(3000);
            startTime=1 ;
            startCAN = 1;
            lcd.clear();
        if (secondsdisplay <= 5) {
        lcd.setCursor(9,0);
        lcd.print("To ");
        if (targetSOC >= 100) {
          targetSOC = 100;
          lcd.print(targetSOC);
          lcd.print("%");
          }
          else {
           lcd.setCursor(12,0);
           lcd.print(" ");
           lcd.print(targetSOC);
           lcd.print("%");
          }
        float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
        float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
        float pv_current = pv_current1 + pv_current2*manualchargercount ;
        if (pv_current >= 10) {   
          float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
          float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
          float pv_current = pv_current1 + pv_current2*manualchargercount ; 
          lcd.setCursor(10,1);
          lcd.print(" ");
          lcd.print(pv_current,1);
         }
         else if (pv_current >= 100) {   
          float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
          float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
          float pv_current = pv_current1 + pv_current2*manualchargercount ; 
          lcd.setCursor(10,1);
          lcd.print(pv_current,1);
         }
        else {
         lcd.setCursor(10,1);
         lcd.print("  ");
         float pv_current1 = (((float)buf1[2]*256.0) + ((float)buf1[3]))/10.0; //highByte/lowByte + offset
         float pv_current2 = (((float)buf2[2]*256.0) + ((float)buf2[3]))/10.0; //highByte/lowByte + offset
         float pv_current = pv_current1 + pv_current2*manualchargercount ;
         lcd.print(pv_current,1);
        }
        lcd.print("A");  
     }
     else {
      if (targetSOC == 30) {
       outputvoltage = 999;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 40) {
       outputvoltage = 1009;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 50) {
       outputvoltage = 1025;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 60) {
       outputvoltage = 1046;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 70) {
       outputvoltage = 1070;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 80) {
       outputvoltage = 1097;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 90) {
       outputvoltage = 1126;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 95) {
       outputvoltage = 1136;
       maxvoltage = outputvoltage;
      }
      if (targetSOC == 100) {
       outputvoltage = 1162;
       maxvoltage = outputvoltage;
      }
      lcd.setCursor(9,0);
      strbuf[0] = 0b01111110; // forward arrow representing input prompt.
      strbuf[1] = 0;
      lcd.print(strbuf);
      float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
      float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
      float pv_voltagemax = max(pv_voltage1,pv_voltage2);
      float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);
     
      if (maxvoltage/10 >= 100) {          
       lcd.print(maxvoltage/10,1);
       lcd.print("V");
      }
      else {        
        lcd.setCursor(10,0);
        lcd.print(" ");
        lcd.print(maxvoltage/10,1);
        lcd.print("V");
      }
      if (pv_voltage >= 100) {
      lcd.setCursor(10,1);
      float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
      float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
      float pv_voltagemax = max(pv_voltage1,pv_voltage2);
      float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);
      lcd.print(pv_voltage,1);
      lcd.print("V");
      }
      else {
        if (pv_voltage < 10) {
          lcd.setCursor(10,1);
          lcd.print("  ");
          float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
          float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
          float pv_voltagemax = max(pv_voltage1,pv_voltage2);
      float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);       
          lcd.print(pv_voltage,1);
          lcd.print("V");
        }
        else {
         lcd.setCursor(10,1);
         lcd.print(" ");
         float pv_voltage1 = (((float)buf1[0]*256.0) + ((float)buf1[1]))/10.0; //highByte/lowByte + offset
         float pv_voltage2 = (((float)buf2[0]*256.0) + ((float)buf2[1]))/10.0; //highByte/lowByte + offset
         float pv_voltagemax = max(pv_voltage1,pv_voltage2);
      float pv_voltage = max(pv_voltage1/2+pv_voltage2/2,pv_voltagemax);
         lcd.print(pv_voltage,1);
         lcd.print("V");
      }
      }
     }
      
         if (secondsdisplay >= 10) {
          secondsdisplay = 0;     
         }
         //Print kWh
    lcd.setCursor(0,1);
    if (chargedenergy < 10) {
      lcd.print(" ");
      lcd.setCursor(1,1);
      lcd.print(chargedenergy,1);
    }
    else {
      lcd.print(chargedenergy,1);
    }
    lcd.setCursor(4,1);
    lcd.print(" kWh @");
      
    
    //Print Time
    lcd.setCursor(0,0);
    if(hours < 10){
      lcd.print("0");
    }
    lcd.print(hours);
    lcd.print(":");
    lcd.setCursor(3,0);
    if(minutes < 10){
      lcd.print("0");
    }
    lcd.print(minutes);
    lcd.print(":");
    lcd.setCursor(6,0);
    if(seconds < 10){
      lcd.print("0");
    }
    lcd.print(seconds);
          }
          else {
            lcd.print("CAN failed :(");
            startCAN = 0 ;
            startTime = 0 ;
            statusCAN = 0 ;
            }
       if (btn == BUTTON_DOWN_PRESSED || btn == BUTTON_DOWN_LONG_PRESSED) { //|| btn == BUTTON_DOWN_PRESSED || btn == BUTTON_DOWN_LONG_PRESSED || btn == BUTTON_LEFT_PRESSED || btn == BUTTON_LEFT_LONG_PRESSED || btn == BUTTON_RIGHT_PRESSED || btn == BUTTON_RIGHT_LONG_PRESSED) {
             startTime = 0;
             lcd.clear();
              return complete;
            }
       }
       
       break;

    default:
    break;
   }
   
  return complete;
   
}


//----------------------------------------------------------------------
// Callback to convert button press to navigation action.
byte getNavAction()
{
  byte navAction = 0;
  byte currentItemHasChildren = Menu1.currentItemHasChildren();
  
  if (btn == BUTTON_UP_PRESSED || btn == BUTTON_UP_LONG_PRESSED) navAction = MENU_ITEM_PREV;
  else if (btn == BUTTON_DOWN_PRESSED || btn == BUTTON_DOWN_LONG_PRESSED) navAction = MENU_ITEM_NEXT;
  else if (btn == BUTTON_SELECT_PRESSED || (btn == BUTTON_RIGHT_PRESSED && currentItemHasChildren)) navAction = MENU_ITEM_SELECT;
  //else if (btn == BUTTON_LEFT_PRESSED) navAction = MENU_BACK;
  return navAction;
}


//----------------------------------------------------------------------
const char EmptyStr[] = "";

// Callback to refresh display during menu navigation, using parameter of type enum DisplayRefreshMode.
void refreshMenuDisplay (byte refreshMode)
{
  char nameBuf[LCD_COLS+1];

/*
  if (refreshMode == REFRESH_DESCEND || refreshMode == REFRESH_ASCEND)
  {
    byte menuCount = Menu1.getMenuItemCount();
    
    // uncomment below code to output menus to serial monitor
    if (Menu1.currentMenuHasParent())
    {
      Serial.print("Parent menu: ");
      Serial.println(Menu1.getParentItemName(nameBuf));
    }
    else
    {
      Serial.println("Main menu:");
    }
    
    for (int i=0; i<menuCount; i++)
    {
      Serial.print(Menu1.getItemName(nameBuf, i));

      if (Menu1.itemHasChildren(i))
      {
        Serial.println("->");
      }
      else
      {
        Serial.println();
      }
    }
  }
*/

  lcd.setCursor(0, 0);
  if (Menu1.currentItemHasChildren())
  {
    rpad(strbuf, Menu1.getCurrentItemName(nameBuf));
    strbuf[LCD_COLS-1] = 0b01111110;            // Display forward arrow if this menu item has children.
    lcd.print(strbuf);
    lcd.setCursor(0, 1);
    lcd.print(rpad(strbuf, EmptyStr));          // Clear config value in display
  }
  else
  {
    byte cmdId;
    rpad(strbuf, Menu1.getCurrentItemName(nameBuf));
    
    if ((cmdId = Menu1.getCurrentItemCmdId()) == 0)
    {
      strbuf[LCD_COLS-1] = 0b01111111;          // Display back arrow if this menu item ascends to parent.
      lcd.print(strbuf);
      lcd.setCursor(0, 1);
      lcd.print(rpad(strbuf, EmptyStr));        // Clear config value in display.
    }
    else
    {
      lcd.print(strbuf);
      lcd.setCursor(0, 1);
      lcd.print(" ");
      
      // TODO Display config value.
     
    }
  }
}
