#include "WiFiEsp.h"
#include "WiFiEspUdp.h"
#include <Time.h>
#include <TimeLib.h>

char ssid[] = "";            // your network SSID (name)
char pass[] = "";        // your network password

int status = WL_IDLE_STATUS;     // the Wifi radio's status

char timeServer[] = "time.nist.gov";  // NTP server
unsigned int localPort = 2390;        // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48;  // NTP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT = 2000;    // timeout in miliseconds to wait for an UDP packet to arrive

byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

unsigned long epoch;
unsigned long startMs, displayMillis;
bool timeupdate = false;
// A UDP instance to let us send and receive packets over UDP
WiFiEspUDP Udp;

int h_x, h_y, m_x, m_y;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.

  pinMode(A9, OUTPUT);
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(A12, OUTPUT);

  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);

  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  Udp.begin(localPort);
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  startMs = millis();
  displayMillis = millis();
  while (!Udp.available() && (millis() - startMs) < UDP_TIMEOUT) {} //wait for NTP reply

  Serial.println(Udp.parsePacket());
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it into the buffer
    Udp.read(packetBuffer, NTP_PACKET_SIZE);

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second

    // set the time
    setTime(((epoch  % 86400L) / 3600) + 3, ((epoch  % 3600) / 60), (epoch % 60), 24, 6, 22); //hour, minute, second, day, month, year

    timeupdate = true;
  }
}



// the loop function runs over and over again forever
void loop() {
  Serial.println(".");
  //animation
  for (int x = 0; x <= hour(); x++) { //start from 0 to the current hour
    h_x = x / 10; //get the digits of x
    h_y = x % 10;
    for (int y = 0; y <= 15; y++) { //400ms
      display_number(h_x); //display the hour digit
      digitalWrite(7, HIGH);
      delay(1);
      digitalWrite(7, LOW);
      delay(1);
      display_number(h_y); //display the hour digit
      digitalWrite(6, HIGH);
      delay(1);
      digitalWrite(6, LOW);
      delay(1);
    }
  }

  for (int x = 0; x <= minute(); x++) {
    m_x = x / 10; //get the digits of x
    m_y = x % 10;
    for (int y = 0; y <= 5; y++) { //400ms

      h_x = hour() / 10;
      h_y = hour() % 10;

      display_number(h_x); //display the hour digit
      digitalWrite(7, HIGH);
      delay(1);
      digitalWrite(7, LOW);
      delay(1);
      display_number(h_y); //display the hour digit
      digitalWrite(6, HIGH);
      delay(1);
      digitalWrite(6, LOW);
      delay(1);

      display_number(m_x); //display the minute digit
      digitalWrite(5, HIGH);
      delay(1);
      digitalWrite(5, LOW);
      delay(1);
      display_number(m_y); //display the minute digit
      digitalWrite(4, HIGH);
      delay(1);
      digitalWrite(4, LOW);
      delay(1);
    }
  }

  displayMillis = millis();

  while ((millis() - displayMillis) < 60000) {

    h_x = hour() / 10;
    h_y = hour() % 10;

    m_x = minute() / 10;
    m_y = minute() % 10;

    display_number(h_x); //display the hour digit
    digitalWrite(7, HIGH);
    delay(1);
    digitalWrite(7, LOW);
    delay(1);

    display_number(h_y); //display the hour digit
    digitalWrite(6, HIGH);
    delay(1);
    digitalWrite(6, LOW);
    delay(1);

    display_number(m_x); //display the minute digit
    digitalWrite(5, HIGH);
    delay(1);
    digitalWrite(5, LOW);
    delay(1);
    display_number(m_y); //display the minute digit
    digitalWrite(4, HIGH);
    delay(1);
    digitalWrite(4, LOW);
    delay(1);
  }

}

void display_number(int number) {
  //Serial.println(number);
  switch (number) {
    case 0:
      digitalWrite(A12, LOW); //A
      digitalWrite(A10, LOW); //B
      digitalWrite(A9, LOW);  //C
      digitalWrite(A11, LOW); //D
      break;
    case 1:
      digitalWrite(A12, HIGH); //A
      digitalWrite(A10, LOW); //B
      digitalWrite(A9, LOW);  //C
      digitalWrite(A11, HIGH); //D
      break;
    case 2:
      digitalWrite(A12, LOW); //A
      digitalWrite(A10, LOW); //B
      digitalWrite(A9, LOW);  //C
      digitalWrite(A11, HIGH); //D
      break;
    case 3:
      digitalWrite(A12, HIGH); //A
      digitalWrite(A10, HIGH); //B
      digitalWrite(A9, HIGH);  //C
      digitalWrite(A11, LOW); //D
      break;
    case 4:
      digitalWrite(A12, LOW); //A
      digitalWrite(A10, HIGH); //B
      digitalWrite(A9, HIGH);  //C
      digitalWrite(A11, LOW); //D
      break;
    case 5:
      digitalWrite(A12, HIGH); //A
      digitalWrite(A10, LOW); //B
      digitalWrite(A9, HIGH);  //C
      digitalWrite(A11, LOW); //D
      break;
    case 6:
      digitalWrite(A12, LOW); //A
      digitalWrite(A10, LOW); //B
      digitalWrite(A9, HIGH);  //C
      digitalWrite(A11, LOW); //D
      break;
    case 7:
      digitalWrite(A12, HIGH); //A
      digitalWrite(A10, HIGH); //B
      digitalWrite(A9, LOW);  //C
      digitalWrite(A11, LOW); //D
      break;
    case 8:
      digitalWrite(A12, LOW); //A
      digitalWrite(A10, HIGH); //B
      digitalWrite(A9, LOW);  //C
      digitalWrite(A11, LOW); //D
      break;
    case 9:
      digitalWrite(A12, HIGH); //A
      digitalWrite(A10, LOW); //B
      digitalWrite(A9, LOW);  //C
      digitalWrite(A11, LOW); //D
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(char *ntpSrv)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(ntpSrv, 123); //NTP requests are to port 123

  Udp.write(packetBuffer, NTP_PACKET_SIZE);

  Udp.endPacket();
}
