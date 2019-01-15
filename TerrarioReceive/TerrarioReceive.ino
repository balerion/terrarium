
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//        myData.control bits:
//        
//        bit 0:  tells whetherlighting should be on or off, depending on time of day
//        bit 1:  
//        bit 2:  
//        bit 3:  
//        bit 4:  sprays frog terrarium
//        bit 5:  sprays snake terrarium
//        bit 6:  turns lights on (overrides bit 7)
//        bit 7:  turns lights off
//        
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////// !!! Serial communication to computer interferes electrically with communication to other arduino. !!! ///////////
/////////// !!!                         limit communication to debugging                                      !!! ///////////


#include <SoftEasyTransfer.h>
#include <serLCD.h>
#include <SoftwareSerial.h>
#include "DHT.h"

#define DHTPIN A0     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

static int whitePin=9;
static int redPin=10;
static int bluePin=11;
//static long dimTime=900000L;
static long dimTime=60000L;

static int lightSpeed=6;
//static int lightSpeed=600;

float whiteBrightness=0;
float blueBrightness=0;
float redBrightness=0;


SoftwareSerial mySerial(7, 6);
DHT dht(DHTPIN, DHTTYPE);

SoftEasyTransfer ET; 
serLCD lcd(A5);

const long waitTime = 1000L;
const long readTime = 5000L;
long lastTime = 0;

float humidity = 0;
float temperature = 0;

boolean firstLoop = true;
float i=3000;

byte degree[8] = {
  B00110,
  B00110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

struct RECEIVE_DATA_STRUCTURE{
  int time0;
  int time1;
  int time2;
  byte control;
  float temp;
  float hum;
  boolean ok;
};

RECEIVE_DATA_STRUCTURE receivedData;
RECEIVE_DATA_STRUCTURE tempData;
RECEIVE_DATA_STRUCTURE actData;



void setup(){

  pinMode(9, INPUT); 
  setPwmFrequency(9, 256);
  pinMode(10, INPUT); 
  setPwmFrequency(10, 256);
  pinMode(11, INPUT); 
  setPwmFrequency(11, 256);


  Serial.begin(9600);
  mySerial.begin(9600);
  ET.begin(details(receivedData), &mySerial);
  dht.begin();

  lcd.createChar(1, degree);

  pinMode(A1, OUTPUT);
  lcd.clear();
  delay(10);
  lcd.setCursor(1,3);
  lcd.print("Starting");
  delay(200);

}




void loop() {

  //  lcd.selectLine(1);
  //  lcd.setCursor(1,1);
  //  lcd.print(i/10,1);



  if(firstLoop) lcd.clear();
  delay(10);

  if(ET.receiveData()){
    
    if (tempData.control==receivedData.control) actData=tempData; 
    tempData=receivedData;
    
    
    firstLoop=false;
    //Serial.println(mydata.control, BIN);




    if ((millis()-lastTime)>readTime) {
      digitalWrite(A1,HIGH);
      if ((millis()-lastTime)>readTime+waitTime) {
        //Serial.println("reading DHT22...");
        humidity=dht.readHumidity();
        temperature=dht.readTemperature();
        digitalWrite(A1,LOW);

        lastTime=millis();

        if (humidity>0 && humidity<100) actData.hum=humidity;
        else actData.hum=0;
        if (temperature>0 && temperature<100) actData.temp=temperature;
        else actData.temp=0;

        //        Serial.print(mydata.hum);
        //        Serial.print(", ");
        //        Serial.println(mydata.temp);
      }
    }

    actData.ok=true;
    delay(10);
    //    ET.sendData();
    LCD_printData();

  }

  //  lcd.selectLine(1);
  //  lcd.print(i/dimTime);

  //--------- light override ------------//
  if ((actData.control & B00000001) != 0) {
    i=0;
  }
  if ((actData.control & B00000010) != 0) {
    i=dimTime;
  }

  //--------- dawn and dusk ------------//
  if ((actData.control & 10000000) != 0) {
    if (i<dimTime) {
      i+=lightSpeed/6;
      if (i>(dimTime/3)) i+=lightSpeed;
    }
    whiteBrightness = constrain(255-i*255.0/dimTime,0,255);
    blueBrightness = constrain(255-i*255.0/dimTime,0,255);
    redBrightness = constrain(255-1.666*i*255.0/dimTime-255.0/dimTime/4,0,255);
    delay(1);
  }
  if ((actData.control & 10000000) == 0) {
    if (i>0) {
      i-=lightSpeed/6;
      if (i>(dimTime/3)) i-=lightSpeed;
    }
    whiteBrightness = constrain(255-1.666*i*255.0/dimTime+255.0/dimTime/4,0,255);
    blueBrightness = constrain(255-2*i*2*255.0/dimTime+255.0/dimTime/2,0,255);
    redBrightness = constrain(255-i*255.0/dimTime,0,255);

    delay(1);
  }

  analogWrite(whitePin, whiteBrightness);
  analogWrite(bluePin, whiteBrightness);
  analogWrite(redPin, whiteBrightness);


  //lightStatus=255*i/300.0;
  //if (lightStatus<1000) i++;


  // Alba
  // analogWrite(whitePin,255-constrain(255*(i/300.0),0,255));
  // analogWrite(bluePin,255-constrain(255*(i/300.0),0,255));
  // analogWrite(redPin,255-constrain(255*((i-10)/300.0),0,255));

  // Tramonto
  // analogWrite(whitePin,constrain(255*((i+30)/300.0),0,255));
  // analogWrite(redPin,constrain(255*(i/300.0),0,255));
  // analogWrite(bluePin,constrain(255*((i+60)/300.0),0,255));

  delay(10);
}



void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
    case 1: 
      mode = 0x01; 
      break;
    case 8: 
      mode = 0x02; 
      break;
    case 64: 
      mode = 0x03; 
      break;
    case 256: 
      mode = 0x04; 
      break;
    case 1024: 
      mode = 0x05; 
      break;
    default: 
      return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } 
    else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } 
  else if(pin == 3 || pin == 11) {
    switch(divisor) {
    case 1: 
      mode = 0x01; 
      break;
    case 8: 
      mode = 0x02; 
      break;
    case 32: 
      mode = 0x03; 
      break;
    case 64: 
      mode = 0x04; 
      break;
    case 128: 
      mode = 0x05; 
      break;
    case 256: 
      mode = 0x06; 
      break;
    case 1024: 
      mode = 0x7; 
      break;
    default: 
      return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}



void LCD_printData() {

  lcd.selectLine(1);
  //lcd.print(mydata.control, BIN);
  lcd.setCursor(1,1);
  lcd.print("        ");
  lcd.setCursor(1,9);
  if (actData.time0<10) lcd.print(0);
  lcd.print(actData.time0);
  lcd.print(":");
  if (actData.time1<10) lcd.print(0);
  lcd.print(actData.time1);
  lcd.print(":");
  if (actData.time2<10) lcd.print(0);
  lcd.print(actData.time2);


  lcd.selectLine(2);
  //lcd.clearLine(2);
  lcd.setCursor(2, 1);
  if (humidity<10) lcd.print(" ");
  lcd.print(humidity,1);
  lcd.setCursor(2, 5);
  lcd.print("%RH   ");
  if (temperature<10) lcd.print(" ");
  lcd.setCursor(2, 11);
  lcd.print(temperature,1);
  lcd.setCursor(2, 15);
  lcd.printCustomChar(1);
  lcd.print("C"); 
}
























