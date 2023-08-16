#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h>
#include <EEPROM.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

DS3232RTC myRTC;

// Pin Connections
int fanCounterPin=4;
int fanPin=5;
int ledPin=6;
int sw1=7;
int sw2=8;
int sw3=9;
int sw4=10;
int sw5=12;
int sw6=11;

int sw1Status=1;
int sw2Status=1;
int sw3Status=1;
int sw4Status=1;
int sw5Status=1;
int sw6Status=1;
int sw1StatusOld=1;
int sw2StatusOld=1;
int sw3StatusOld=1;
int sw4StatusOld=1;
int sw5StatusOld=1;
int sw6StatusOld=1;

bool alarmStatus=1;
bool lightStatus=0;


float alarmDuration=1800.0;
float secondsToAlarm;
float delayTime;
int delayTimeInt;
int alarmHour=6;
int alarmMinute=30;

float brightnessNow=0.0;
int fanSpeed=0;
float maxBrightness=100.0;
int maxBrightnessInt=100;
int maxBrightnessLevel=1;
unsigned long lightBeginTime;
unsigned long currentTime;
unsigned long lightTime;

byte alarmChar[8] = {0b00100,0b01110,0b01110,0b01110,0b11111,0b00000,0b00100,0b00000};

void setup()
{
  // initialize the LCD
  lcd.begin();
  lcd.createChar(0, alarmChar);
  

  // Turn on the blacklight and print a message.
  lcd.backlight();


     Serial.begin(115200);
    myRTC.begin();
    setSyncProvider(myRTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");


  pinMode(ledPin,OUTPUT);
  pinMode(fanPin,OUTPUT);
  pinMode(fanCounterPin,INPUT);
  pinMode(sw1,INPUT);
  pinMode(sw2,INPUT);
  pinMode(sw3,INPUT);
  pinMode(sw4,INPUT);
  pinMode(sw5,INPUT);
  pinMode(sw6,INPUT);

  // Read alarm hour and minute from EEPROM
alarmHour=EEPROM.read(0);
alarmMinute=EEPROM.read(1);
if (alarmHour>=24){
  alarmHour=6;
}
if (alarmMinute>=60){
  alarmMinute=30;
}
}

void loop()
{
lcd.createChar(1,alarmChar);

sw1Status=digitalRead(sw1);
sw2Status=digitalRead(sw2);
sw3Status=digitalRead(sw3);
sw4Status=digitalRead(sw4);
sw5Status=digitalRead(sw5);
sw6Status=digitalRead(sw6);


if (sw1StatusOld==0 && sw1Status==1){
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Set Alarm");
  int jSet=1;
  while(jSet==1){

    sw2Status=digitalRead(sw2);
    sw3Status=digitalRead(sw3);
    sw4Status=digitalRead(sw4);

    if (sw2StatusOld==0 && sw2Status==1){
        jSet=0; 
      }

    if (sw3StatusOld==0 && sw3Status==1){

      alarmHour=alarmHour+1;
      if (alarmHour >= 24){
        alarmHour=0;
      }
      
    }


    if (sw4StatusOld==0 && sw4Status==1){
      alarmMinute=alarmMinute+1;
      if (alarmMinute >= 60){
        alarmMinute=0;
      }
      
    }

    


    lcd.setCursor(5,1);
    printDigits(alarmHour);
    lcd.print(":");
    printDigits(alarmMinute);
    sw2StatusOld=sw2Status;
    sw3StatusOld=sw3Status;
    sw4StatusOld=sw4Status;
  }
  EEPROM.update(0,alarmHour);
  EEPROM.update(1,alarmMinute);
  lcd.clear();
  
}


if (sw3StatusOld==0 && sw3Status==1 ){
  maxBrightnessLevel=maxBrightnessLevel+1;
  if (maxBrightnessLevel>=10){
    maxBrightnessLevel=10;
  }
}

if (sw4StatusOld==0 && sw4Status==1 ){
  maxBrightnessLevel=maxBrightnessLevel-1;
  if(maxBrightnessLevel<=1){
    maxBrightnessLevel=1;
  }
}


if (sw5StatusOld==0 && sw5Status==1 ){
  if (alarmStatus==1){
    alarmStatus=0;
    lcd.clear();
  }
  else {
    alarmStatus=1;
    lcd.clear();
    }
}

if (sw6StatusOld==0 && sw6Status==1){
  light();
}

secondsToAlarm=((alarmHour*3600)+(alarmMinute*60)+0)-((hour()*3600)+(minute()*60)+second());
if (alarmStatus==1 && secondsToAlarm==alarmDuration){

  alarmBegin();
  
}

if(maxBrightnessLevel==1){
  maxBrightness=5;
}
if(maxBrightnessLevel==2){
  maxBrightness=35;
}
if(maxBrightnessLevel==3){
  maxBrightness=65;
}
if(maxBrightnessLevel==4){
  maxBrightness=95;
}
if(maxBrightnessLevel==5){
  maxBrightness=125;
}
if(maxBrightnessLevel==6){
  maxBrightness=155;
}
if(maxBrightnessLevel==7){
  maxBrightness=185;
}
if(maxBrightnessLevel==8){
  maxBrightness=210;
}
if(maxBrightnessLevel==9){
  maxBrightness=235;
}
if(maxBrightnessLevel==10){
  maxBrightness=254;
}


maxBrightnessInt=maxBrightness;
printNormal(alarmStatus);

currentTime=millis();
lightTime=currentTime-lightBeginTime;
if (lightStatus==1 && lightTime>=1200000){
  light();
}
if (lightStatus==1){
  brightnessNow=maxBrightnessInt;
}
if (lightStatus==0){
  brightnessNow=0;
}


fan();
analogWrite(ledPin,brightnessNow);
analogWrite(fanPin,fanSpeed);



sw1StatusOld=sw1Status;
sw2StatusOld=sw2Status;
sw3StatusOld=sw3Status;
sw4StatusOld=sw4Status;
sw5StatusOld=sw5Status;
sw6StatusOld=sw6Status;

}

void light(){
  maxBrightnessInt=maxBrightness;
  if(lightStatus==0){
    lightBeginTime=millis();
    int jL1;
    for(jL1=0;jL1<maxBrightnessInt;jL1++){
      brightnessNow=jL1;
      analogWrite(ledPin,brightnessNow);
      delay(12);
    }
    fan();
    analogWrite(ledPin,brightnessNow);
    analogWrite(fanPin,fanSpeed);
    lightStatus=1;
    return;
  }
  if(lightStatus==1){
    while(brightnessNow>=1){
      brightnessNow=brightnessNow-1;
      analogWrite(ledPin,brightnessNow);
      delay(12);
    }

    fan();
    analogWrite(ledPin,brightnessNow);
    analogWrite(fanPin,fanSpeed);
    lightStatus=0;
    return;
  }
}

void alarmBegin(){
  lcd.clear();
  int jAl=1;
  while(jAl==1){
    
    sw5Status=digitalRead(sw5);
    if (sw5StatusOld==0 && sw5Status==1 ){
      jAl=0;
    }
    if (secondsToAlarm<=-2700){
      jAl=0;
    }
    secondsToAlarm=((alarmHour*3600)+(alarmMinute*60)+0)-((hour()*3600)+(minute()*60)+second());
    
    float alarmFullBritnessDuration=alarmDuration/3;
    if((secondsToAlarm < alarmDuration) && (secondsToAlarm > alarmFullBritnessDuration) ){
    brightnessNow=254-(254*((secondsToAlarm-alarmFullBritnessDuration)/(alarmDuration-alarmFullBritnessDuration)));

   }
   if(secondsToAlarm < alarmFullBritnessDuration ){
    brightnessNow=254;
    }
    fan(); 
    lcd.setCursor(6,0);
    lcd.print("Good");
    lcd.setCursor(5,1);
    lcd.print("Morning!");
    sw5StatusOld=sw5Status;
    analogWrite(ledPin,brightnessNow);
    analogWrite(fanPin,fanSpeed);
  
  }
  lcd.clear();
  lightStatus=1;
  light();
}

void fan(){
  if (brightnessNow<=50){
    fanSpeed=0;
  }
    if (brightnessNow>50 && brightnessNow<=70){
    fanSpeed=60;
  }
  if (brightnessNow>70 && brightnessNow<=110){
    fanSpeed=95;
  }
  if (brightnessNow>110 && brightnessNow<=150){
    fanSpeed=125;
  }
  if (brightnessNow>150 && brightnessNow<=190){
    fanSpeed=175;
  }
  if (brightnessNow>190 && brightnessNow<=220){
    fanSpeed=200;
  }
  if (brightnessNow>220 && brightnessNow<=235){
    fanSpeed=230;
  }
  if (brightnessNow>235){
    fanSpeed=254;
}
  
}

void printNormal(int Status){

if(Status==1){
lcd.setCursor(0,0);
printDigits(hour());
lcd.print(":");
printDigits(minute());



  lcd.setCursor(10,0);
  lcd.write(1);
  printDigits(alarmHour);
  lcd.print(":");
  printDigits(alarmMinute);




lcd.setCursor(0,1);
printDigits(day());
lcd.print(".");
printDigits(month());
lcd.print(".");
int year2=year()-2000;
lcd.print(year2);
lcd.setCursor(15,1);
lcd.print(maxBrightnessLevel);
}


if(Status==0){
lcd.setCursor(0,0);
printDigits(hour());
lcd.print(":");
printDigits(minute());



lcd.setCursor(0,1);
printDigits(day());
lcd.print(".");
printDigits(month());
lcd.print(".");
int year2=year()-2000;
lcd.print(year2);
lcd.setCursor(15,1);
lcd.print(maxBrightnessLevel);

}

}

void printDigits(int digits)
{
    // utility function for digital clock display: prints leading 0
    if(digits < 10)
        lcd.print('0');
    lcd.print(digits);
}
