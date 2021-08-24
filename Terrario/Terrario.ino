//#include <SPI.h>
//#include <Ethernet.h>
//#include <HttpClient.h>
#include <ArduinoJson.h>
//#include <avr/wdt.h>
//#include <Xively.h>
#include <SoftEasyTransfer.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(A4, A5);


long commTime = 0;
long commInterval = 10;


const unsigned long halfhour = 1800000;
const unsigned long spraytime = 2000;

float i = 0;
int Fuso = 0; //0 per ora solare
//int Fuso = -1; //-1 per ora legale
boolean on = false;
boolean called2 = false;
long timercounter2 = 0;
boolean called4 = false;
long timercounter4 = 0;
long dimTime = 900000L;
boolean override = false;

int redLight = 3;
int blueLight = 9;
int whiteLight = 6;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

//unsigned int localPort = 8888;      // local port to listen for UDP packets
//IPAddress timeServer(193, 204, 114, 232); // time.nist.gov NTP server
//IPAddress timeServer(216,239,35,8); // time.google.com NTP server
//const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
//byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
//EthernetUDP Udp; // A UDP instance to let us send and receive packets over UDP
long lastNtpRetrievalTime = 0;        // last time you connected to the NTP server, in milliseconds
const int retrievalInterval = 20000;  //delay between updates to NTP server
int time[3];
long second = 0;


//IPAddress ip(10, 0, 1, 20); // fill in an available IP address on your network here, for manual configuration:
//EthernetClient client; // initialize the library instance:
//long lastConnectionTime = 0;        // last time you connected to the server, in milliseconds
//boolean lastConnected = false;      // state of the connection last time through the main loop
//const int postingInterval = 15000;  //delay between updates to Pachube.com


// Your Xively key to let you upload data
//char xivelyKey[] = "h5nlzsKtBl7r2nKQUQtP22Vh4W3SrNaReYnFGf7TLi6pLyez";
// Define the strings for our datastream IDs
char tempID[] = "Temperature";
char humID[] = "Humidity";
const int bufferSize = 140;
char bufferValue[bufferSize]; // enough space to store the string we're going to send
//XivelyDatastream datastreams[] = {
//  XivelyDatastream(tempID, strlen(tempID), DATASTREAM_FLOAT),
//  XivelyDatastream(humID, strlen(humID), DATASTREAM_FLOAT),
//};
// Finally, wrap the datastreams into a feed
//XivelyFeed feed(511640911, datastreams, 2 /* number of datastreams */);
//XivelyClient xivelyclient(client);


//create object
SoftEasyTransfer ET;

struct SEND_DATA_STRUCTURE {
  int time0;
  int time1;
  int time2;
  byte control;
  float temp;
  float hum;
  boolean ok;
};

//give a name to the group of data
SEND_DATA_STRUCTURE mydata;


void setup()
{
  //wdt_enable(WDTO_4S);

  analogReference(INTERNAL);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);

  mySerial.begin(9600);
  //start the library, pass in the data details and the name of the serial port.
  ET.begin(details(mydata), &mySerial);


  Serial.begin(9600);
  Serial.println("Welcome to the Terrarium Interface");
  // start the Ethernet and UDP connections:
  delay(1000);   // give the ethernet module time to boot up:
  // while (Ethernet.begin(mac) != 1)
  // {
  //   Serial.println("Error getting IP address via DHCP, trying again...");
  //   delay(15000);
  // }
  // Udp.begin(localPort);

  // GetTime();
}


void loop() {


  //------------- Serial input for overrides   -------------//
  while (Serial.available() > 0) {

    // create json buffer
    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, Serial);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // Fetch values.
    //
    // Most of the time, you can rely on the implicit casts.
    // In other case, you can do doc["time"].as<long>();
    const char* sensor = doc["sensor"];
    long timeRPI = doc["time"];
    double latitude = doc["data"][0];
    double longitude = doc["data"][1];
    //    Serial.println(time);

    //    byte test = Serial.parseInt();

    time[0] = ((timeRPI + 7200)  % 86400L) / 3600; // hours
    time[1] = ((timeRPI + 7200)  % 3600) / 60;   // minutes
    time[2] = (timeRPI + 7200) % 60;             // seconds


    byte test = int(longitude) & B00001111;

    //    if (Serial.read() == '\n') {
    Serial.print(test);
    Serial.print(", ");
    Serial.print(test, BIN);
    mydata.control = (mydata.control & B11110000);
    mydata.control = (mydata.control | test);
    Serial.print(", ");
    Serial.println(mydata.control, BIN);

    //    }
  }



  //watchdog timer reset
  //wdt_reset();

  // Recuperare l'ora
  if ((millis() - lastNtpRetrievalTime) > retrievalInterval) {
    //   GetTime();
    lastNtpRetrievalTime = millis(); // wait 20 seconds before asking for the time again

    // Printing retrieved time
    Serial.print(time[0]);
    Serial.print(":");
    Serial.print(time[1]);
    Serial.print(":");
    Serial.println(time[2]);

  }
  updateTime();

  //if((millis() - lastConnectionTime) > postingInterval) {
  //datastreams[0].setFloat(mydata.temp);
  //datastreams[1].setFloat(mydata.hum);

  //Serial.print("Read sensor value ");
  //Serial.println(datastreams[0].getFloat());

  //Serial.print("Read sensor value ");
  //Serial.println(datastreams[1].getFloat());

  //Serial.println("Uploading it to Xively");
  //int ret = xivelyclient.put(feed, xivelyKey);
  //Serial.print("xivelyclient.put returned ");
  //Serial.println(ret);

  //Serial.println();

  //lastConnectionTime=millis();

  //}

  // Gestione luci
  if (on && i < dimTime) {
    i += 1;
    delay(1);
  }
  if (time[0] >= 8 && time[0] <= 24) {
    on = true;
    mydata.control = (mydata.control & B01111111) | B10000000;

    //analogWrite(whiteLight,(255-i*255.0/dimTime));
    //analogWrite(blueLight,(255-i*255.0/dimTime));
    //analogWrite(redLight,(255-i*255.0/dimTime));
  }
  else {
    on = false;
    mydata.control = (mydata.control & B01111111);

    //analogWrite(whiteLight,(255-i*255.0/dimTime));
    //analogWrite(blueLight,(255-i*255.0/dimTime));
    //analogWrite(redLight,(255-i*255.0/dimTime));
    //if (i>0) {
    //  i-=1;
    //  delay(1);
    //}
  }

  //if (time[0]>=9 && time[0]<=23) {
  //  i=dimTime;
  //}
  //if (time[0]>=1 && time[0]<=8) {
  //  i=0;
  //}


  // Gestione umidificazione
  if (time[1] == 1 || time[1] == 30) {
    if (time[0] >= 11 && time[0] <= 22) {
      spray2(10000);
    }
  }

  if (time[1] == 1) {
    if (time[0] >= 11 && time[0] <= 22) {
      spray4(10000);
    }
  }


  //--------- spray override ------------//

  if ((mydata.control & B00000100) != 0) {
    override = true;
    digitalWrite(4, LOW);
  }

  if ((mydata.control & B00001000) != 0) {
    override = true;
    digitalWrite(2, LOW);
  }

  if ((mydata.control & B00001100) == 0) {
    override = false;
  }



  //--------- spray failsafe checks ---------//

  if (time[1] != 1 && time[1] != 30 && !override) {
    digitalWrite(4, HIGH);
    digitalWrite(2, HIGH);
  }


  // Communication to other Arduino
  mydata.time0 = time[0];
  mydata.time1 = time[1];
  mydata.time2 = time[2];

  if (millis() - commTime > commInterval) {
    ET.sendData();
    // give it time to respond
    delay(10);
    ET.receiveData();

    //Serial.println(mydata.hum);
    mydata.ok = false;
    commTime = millis();
  }


  /*spray(4);
    spray(2);
    delay(1800000);

    spray(2);
    delay(1800000);*/

  // read the analog sensor:
  /*int sensorReading = analogRead(A0);

    // if there's incoming data from the net connection.
    // send it out the serial port.  This is for debugging
    // purposes only:
    if (client.available()) {
    char c = client.read();
    Serial.print(c);
    }

    // if there's no net connection, but there was one last time
    // through the loop, then stop the client:
    if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    }

    // if you're not connected, and ten seconds have passed since
    // your last connection, then connect again and send data:
    if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    sendData(sensorReading);
    }
    // store the state of the connection for next time through
    // the loop:
    lastConnected = client.connected();*/



}


void spray2(int time2) {

  if (!called2 | override) {
    timercounter2 = millis();
  }
  if ((millis() - timercounter2) < time2) {
    digitalWrite(2, LOW);
    called2 = true;
  }
  else {
    digitalWrite(2, HIGH);
  }
  if ((millis() - timercounter2) > 300000) {
    called2 = false;
  }
}

void spray4(int time4) {

  if (!called4 | override) {
    Serial.println(override);
    timercounter4 = millis();
  }
  if ((millis() - timercounter4) < time4) {
    digitalWrite(4, LOW);
    called4 = true;
  }
  else {
    digitalWrite(4, HIGH);
  }
  if ((millis() - timercounter4) > 300000) {
    called4 = false;
  }
}


// void GetTime() {
//   sendNTPpacket(timeServer); // send an NTP packet to a time server

//   // wait to see if a reply is available
//   delay(1000);
//   if ( Udp.parsePacket() ) {

//     // We've received a packet, read the data from it
//     Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

//     //the timestamp starts at byte 40 of the received packet and is four bytes,
//     // or two words, long. First, esxtract the two words:

//     unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
//     unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
//     // combine the four bytes (two words) into a long integer
//     // this is NTP time (seconds since Jan 1 1900):
//     unsigned long secsSince1900 = highWord << 16 | lowWord;

//     // now convert NTP time into everyday time:
//     // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
//     const unsigned long seventyYears = 2208988800UL;
//     // subtract seventy years:
//     unsigned long epoch = secsSince1900 - seventyYears;

//     epoch += 7200;
//     time[0] = (epoch  % 86400L) / 3600; // hours
//     time[1] = (epoch  % 3600) / 60;     // minutes
//     time[2] = epoch % 60;               // seconds

//     // Fuso orario
//     if (time[0] >= (-Fuso)) {
//       time[0] += Fuso;
//     }
//     else {
//       time[0] = time[0] + 24 + Fuso;
//     }

//   }




// }

// // send an NTP request to the time server at the given address
// unsigned long sendNTPpacket(IPAddress& address)
// {
//   // set all bytes in the buffer to 0
//   memset(packetBuffer, 0, NTP_PACKET_SIZE);
//   // Initialize values needed to form NTP request
//   // (see URL above for details on the packets)
//   packetBuffer[0] = 0b11100011;   // LI, Version, Mode
//   packetBuffer[1] = 0;     // Stratum, or type of clock
//   packetBuffer[2] = 6;     // Polling Interval
//   packetBuffer[3] = 0xEC;  // Peer Clock Precision
//   // 8 bytes of zero for Root Delay & Root Dispersion
//   packetBuffer[12]  = 49;
//   packetBuffer[13]  = 0x4E;
//   packetBuffer[14]  = 49;
//   packetBuffer[15]  = 52;

//   // all NTP fields have been given values, now
//   // you can send a packet requesting a timestamp:
//   if (Udp.beginPacket(address, 123) == 1) {
//     Udp.write(packetBuffer, NTP_PACKET_SIZE);
//     if (Udp.endPacket() != 1) Serial.println(F("Send error"));
//   }
//   else {
//     Serial.println(F("Socket error"));
//   }
// }

// this method makes a HTTP connection to the server:
//void sendData(int thisData) {
//  // if there's a successful connection:
//  if (client.connect("www.pachube.com", 80)) {
//    Serial.println("connecting...");
//    // send the HTTP PUT request.
//    // fill in your feed address here:
//    client.print("PUT /api/YOUR_FEED_HERE.csv HTTP/1.1\n");
//    client.print("Host: www.pachube.com\n");
//    // fill in your Pachube API key here:
//    client.print("X-PachubeApiKey: YOUR_KEY_HERE\n");
//    client.print("Content-Length: ");
//
//    // calculate the length of the sensor reading in bytes:
//    int thisLength = getLength(thisData);
//    client.println(thisLength, DEC);
//
//    // last pieces of the HTTP PUT request:
//    client.print("Content-Type: text/csv\n");
//    client.println("Connection: close\n");
//
//    // here's the actual content of the PUT request:
//    client.println(thisData, DEC);
//
//    // note the time that the connection was made:
//    lastConnectionTime = millis();
//  }
//  else {
//    // if you couldn't make a connection:
//    Serial.println("connection failed");
//  }
//}


// This method calculates the number of digits in the
// sensor reading.  Since each digit of the ASCII decimal
// representation is a byte, the number of digits equals
// the number of bytes:

int getLength(int someValue) {
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten,
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue / 10;
  while (dividend > 0) {
    dividend = dividend / 10;
    digits++;
  }
  // return the number of digits:
  return digits;
}


void updateTime() {
  if ((millis() - second) > 1000) {
    time[2] ++;
    if (time[2] > 59) {
      time[2] = 0;
      time[1] ++;
      if (time[1] > 59) {
        time[1] = 0;
        time[0] ++;
        if (time[0] > 23) {
          time[0] = 0;
        }
      }
    }
    second = millis();
  }
}
