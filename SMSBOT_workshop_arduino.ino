#include "Adafruit_FONA.h"
#define FONA_RX 4
#define FONA_TX 5
#define FONA_RST 6

//Adafruit_PCD8544 display = Adafruit_PCD8544(9, 8, 7, 6, 5);
// this is a large buffer for replies
char replybuffer[355];

#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
int pwrPin = 3;
//byte []6
// Use this for FONA 800 and 808s
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
// Use this one for FONA 3G
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;
String buf = "";
char nSMS[140];
String mode = "smsmode";

void setup() {
  pinMode(pwrPin,OUTPUT);
  digitalWrite(pwrPin, HIGH);
  while (!Serial);

  Serial.begin(9600);
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(9600);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default:
      Serial.println(F("???")); break;
  }

  // Print module IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }
  printMenu();
  
}

void printMenu(void) {
  Serial.println(F("-------------------------------------"));
  Serial.println(F("[?] Print this menu"));
  Serial.println(F("[b] read the Battery V and % charged"));
  Serial.println(F("[C] read the SIM CCID"));
  Serial.println(F("[i] read RSSI"));
  Serial.println(F("[n] get Network status"));
  Serial.println(F("[c] make phone Call"));
  Serial.println(F("[h] Hang up phone"));
  Serial.println(F("[p] Pick up phone"));
  // SMS
  Serial.println(F("[N] Number of SMSs"));
  Serial.println(F("[r] Read SMS #"));
  Serial.println(F("[R] Read All SMS"));
  Serial.println(F("[d] Delete SMS #"));
  Serial.println(F("[D] Delete All SMS"));
  Serial.println(F("[s] Send SMS"));
  // Time
  Serial.println(F("[y] Enable network time sync (FONA 800 & 808)"));
  Serial.println(F("[Y] Enable NTP time sync (GPRS FONA 800 & 808)"));
  Serial.println(F("[t] Get network time"));
  // GPRS
  Serial.println(F("[G] Enable GPRS"));
  Serial.println(F("[g] Disable GPRS"));
  Serial.println(F("[l] Query GSMLOC (GPRS)"));
  // GPS
  Serial.println(F("-------------------------------------"));
  Serial.println(F(""));

}
void loop() {
  Serial.print(F("FONA> "));
  while (! Serial.available() ) {
    //Serial.pritln()
    if (fona.available()) {
      // Strin Serial.println
      char ms = char(fona.read());
      if (ms != '\n') {
        buf += ms;
      } else {
        if (buf.substring(1, 5) == "CMTI") {
          Serial.println("New Message received");
          String idMensaje = buf.substring(12);
          idMensaje.trim();
          Serial.print("Id del mensaje = ");
          Serial.println (idMensaje);
          newMessage(idMensaje);
        }

        Serial.println(buf);
        buf = "";
      }


    }
  }
  String line = Serial.readStringUntil('\n');
  line.trim();
  Serial.println(line);
  if (line.equals( "commands")) {
    mode = "commands";
    Serial.println("Estoy en el bloque de comandos");
  } else if (line.equals("smsmode")) {
    mode = "smsmode";
    Serial.println("Estoy en el bloque de SMS");
  }

  if (mode == "commands") {
    if (line.length() < 2) {
      char  buff [line.length()+1];
      line.toCharArray(buff, line.length()+1);
      char command = buff[0];
      Serial.print("comando = ");Serial.println(command);
      switch (command) {
        case '?': {
            printMenu();
            break;
          }
        case 'b': {
            // read the battery voltage and percentage
            uint16_t vbat;
            if (! fona.getBattVoltage(&vbat)) {
              Serial.println(F("Failed to read Batt"));
            } else {
              Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
            }
            if (! fona.getBattPercent(&vbat)) {
              Serial.println(F("Failed to read Batt"));
            } else {
              Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
            }
            break;
          }
        case 'C': {
            // read the CCID
            fona.getSIMCCID(replybuffer);  // make sure replybuffer is at least 21 bytes!
            Serial.print(F("SIM CCID = ")); Serial.println(replybuffer);
            break;
          }
        case 'i': {
            // read the RSSI
            uint8_t n = fona.getRSSI();
            int8_t r;
            Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
            if (n == 0) r = -115;
            if (n == 1) r = -111;
            if (n == 31) r = -52;
            if ((n >= 2) && (n <= 30)) {
              r = map(n, 2, 30, -110, -54);
            }
            Serial.print(r); Serial.println(F(" dBm"));
            break;
          }
        case 'n': {
            // read the network/cellular status
            uint8_t n = fona.getNetworkStatus();
            Serial.print(F("Network status "));
            Serial.print(n);
            Serial.print(F(": "));
            if (n == 0) Serial.println(F("Not registered"));
            if (n == 1) Serial.println(F("Registered (home)"));
            if (n == 2) Serial.println(F("Not registered (searching)"));
            if (n == 3) Serial.println(F("Denied"));
            if (n == 4) Serial.println(F("Unknown"));
            if (n == 5) Serial.println(F("Registered roaming"));
            break;
          }
        /*** Call ***/
        case 'c': {
            // call a phone!
            char number[30];
            flushSerial();
            Serial.print(F("Call #"));
            readline(number, 30);
            Serial.println();
            Serial.print(F("Calling ")); Serial.println(number);
            if (!fona.callPhone(number)) {
              Serial.println(F("Failed"));
            } else {
              Serial.println(F("Sent!"));
            }
            break;
          }
        case 'h': {
            // hang up!
            if (! fona.hangUp()) {
              Serial.println(F("Failed"));
            } else {
              Serial.println(F("OK!"));
            }
            break;
          }
        case 'p': {
            // pick up!
            if (! fona.pickUp()) {
              Serial.println(F("Failed"));
            } else {
              Serial.println(F("OK!"));
            }
            break;
          }
        /*** SMS ***/

        case 'N': {
            // read the number of SMS's!
            int8_t smsnum = fona.getNumSMS();
            if (smsnum < 0) {
              Serial.println(F("Could not read # SMS"));
            } else {
              Serial.print(smsnum);
              Serial.println(F(" SMS's on SIM card!"));
            }
            break;
          }
        case 'r': {
            // read an SMS
            flushSerial();
            Serial.print(F("Read #"));
            uint8_t smsn = readnumber();
            Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
            // Retrieve SMS sender address/phone number.
            if (! fona.getSMSSender(smsn, replybuffer, 250)) {
              Serial.println("Failed!");
              break;
            }
            Serial.print(F("FROM: ")); Serial.println(replybuffer);
            // Retrieve SMS value.
            uint16_t smslen;
            if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
              Serial.println("Failed!");
              break;
            }
            Serial.print(F("***** SMS #")); Serial.print(smsn);
            Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
            Serial.println(replybuffer);
            Serial.println(F("*****"));
            break;
          }
        case 'R': {
            // read all SMS
            int8_t smsnum = fona.getNumSMS();
            uint16_t smslen;
            int8_t smsn;
            if ( (type == FONA3G_A) || (type == FONA3G_E) ) {
              smsn = 0; // zero indexed
              smsnum--;
            } else {
              smsn = 1;  // 1 indexed
            }
            for ( ; smsn <= smsnum; smsn++) {
              Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
              if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
                Serial.println(F("Failed!"));
                break;
              }
              // if the length is zero, its a special case where the index number is higher
              // so increase the max we'll look at!
              if (smslen == 0) {
                Serial.println(F("[empty slot]"));
                smsnum++;
                continue;
              }
              Serial.print(F("***** SMS #")); Serial.print(smsn);
              Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
              Serial.println(replybuffer);
              Serial.println(F("*****"));
            }
            break;
          }

        case 'd': {
            // delete an SMS
            flushSerial();
            Serial.print(F("Delete #"));
            uint8_t smsn = readnumber();
            Serial.print(F("\n\rDeleting SMS #")); Serial.println(smsn);
            if (fona.deleteSMS(smsn)) {
              Serial.println(F("OK!"));
            } else {
              Serial.println(F("Couldn't delete"));
            }
            break;
          }
        case 'D': {
            deleteAllSMS();
            break;
          }

        case 's': {
            // send an SMS!
            char sendto[21], message[141];
            flushSerial();
            Serial.print(F("Send to #"));
            readline(sendto, 20);
            Serial.println(sendto);
            Serial.print(F("Type out one-line message (140 char): "));
            readline(message, 140);
            Serial.println(message);
            if (!fona.sendSMS(sendto, message)) {
              Serial.println(F("Failed"));
            } else {
              Serial.println(F("Sent!"));
            }
            break;
          }

        case 't': {
            // read the time
            char buffer[23];
            fona.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
            Serial.print(F("Time = ")); Serial.println(buffer);
            break;
          }
        /*********************************** GPRS */
        case 'g': {
            // turn GPRS off
            if (!fona.enableGPRS(false))
              Serial.println(F("Failed to turn off"));
            break;
          }
        case 'G': {
            // turn GPRS on
            if (!fona.enableGPRS(true))
              Serial.println(F("Failed to turn on"));
            break;
          }
        case 'l': {
            // check for GSMLOC (requires GPRS)
            uint16_t returncode;
            if (!fona.getGSMLoc(&returncode, replybuffer, 250))
              Serial.println(F("Failed!"));
            if (returncode == 0) {
              Serial.println(replybuffer);
            } else {
              Serial.print(F("Fail code #")); Serial.println(returncode);
            }
            break;
          }
        default: {
            Serial.println(F("Unknown command"));
            printMenu();
            break;
          }
      }

      // flush input
      flushSerial();
      while (fona.available()) {
        Serial.write(fona.read());
      }
    }





    //Serial.println("command = "+buff);



  } else {
    //line.trim();
    //msg = '^'+num+'|'+message; 
    int indexSeparator = 0;
    char  buff [line.length()+1];
    line.toCharArray(buff, line.length()+1);
    char firstChar = buff[0];
    Serial.println(firstChar);
    if(firstChar =='^'){
      for(int i = 1; i<sizeof(buff); i++){
        if(buff[i]=='|'){
         indexSeparator = i;  
        }
        
      }
      String num = line.substring(1, indexSeparator);
      String msg = line.substring(indexSeparator+1, line.length() );
      Serial.println(num);
      Serial.println(msg);
      sendSMS2(num,msg);
    }
    
    //
  }
}
boolean newMessage(String id) {
  flushSerial();
  
  /////Serial.println("nuevo Mensaje");
  int smsn = id.toInt();
  /////Serial.println(smsn);
  char sender [21];
  if (! fona.getSMSSender(smsn, sender, 250)) {
    Serial.println("Failed getting Sender Number!");
    return false;
  }
  ////////Serial.print(F("FROM: ")); Serial.println(sender);
  String sender2 = sender;
  // Retrieve SMS value.
  uint16_t smslen;
  if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
    Serial.println("Failed reading SMS!");
    return false;
  }
  Serial.print(F("***** SMS #")); Serial.print(smsn);
  Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
  String temp = replybuffer;
  String temp2 = utf8ascii(temp);
  Serial.println(temp2);
 
 //////////////////send message to PI
  //String mensajeFinal = '%' + sender2 + '|' + temp2;
  //Serial.println(mensajeFinal);
  /////////////////send mirror response
   String msn = replybuffer;
  sendSMS2(sender2, msn);
  //////
  int8_t smsnum = fona.getNumSMS();
  if (smsnum > 25) {
    deleteAllSMS();
  }

}
void sendSMS2(String sender, String mess) {
  //char sendto[21], message[141];
  char sendto[21];
  sender.toCharArray(sendto, 21);
  char message[141];
  mess.toCharArray(message, 141);

  flushSerial();
  Serial.print(F("Send to #"));

  Serial.println(sendto);
  Serial.print(F("Type out one-line message (140 char): "));
  Serial.println(message);
  if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("Sent!"));
  }
}

void deleteAllSMS() {

  int8_t smsnum = fona.getNumSMS();
  for (int i = 0; i < smsnum; i++) {
    if (fona.deleteSMS(i + 1)) {
      Serial.print(F("\n\rDeleting SMS #")); Serial.println(i + 1);
      Serial.println(F("OK!"));
    } else {
      Serial.println(F("Couldn't delete"));
    }
  }
}
void flushSerial() {
  while (Serial.available())
    Serial.read();
}
/*
void writeMessageToLCD(String ms){
 display.clearDisplay();
  display.setCursor(0,0);
  display.println(ms);
  display.display();
}*/
char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
static byte c1;  // Last character buffer

byte utf8ascii(byte ascii) {
  if ( ascii < 128 ) // Standard ASCII-set 0..0x7F handling
  { c1 = 0;
    return ( ascii );
  }
  // get previous input
  byte last = c1;   // get last char
  c1 = ascii;       // remember actual character
  switch (last)     // conversion depending on first UTF8-character
  { case 0xC2: return  (ascii);  break;
    case 0xC3: return  (ascii | 0xC0);  break;
    case 0x82: if (ascii == 0xAC) return (0x80);   // special case Euro-symbol
  }
  return  (0);                                     // otherwise: return zero, if character has to be ignored
}

String utf8ascii(String s)
{
  String r = "";
  char c;
  for (int i = 0; i < s.length(); i++)
  {
    c = utf8ascii(s.charAt(i));
    if (c != 0) r += c;
  }
  return r;
}

void utf8ascii(char* s)
{
  int k = 0;
  char c;
  for (int i = 0; i < strlen(s); i++)
  {
    c = utf8ascii(s[i]);
    if (c != 0)
      s[k++] = c;
  }
  s[k] = 0;
}
