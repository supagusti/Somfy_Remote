/*   This sketch allows you to emulate a Somfy RTS or Simu HZ remote.
   If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/

   The rolling code will be stored in EEPROM, so that you can power the Arduino off.

   Easiest way to make it work for you:
    - Choose a remote number
    - Choose a starting point for the rolling code. Any unsigned int works, 1 is a good start
    - Upload the sketch
    - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
    - send 'p' to the serial terminal
  To make a group command, just repeat the last two steps with another blind (one by one)

  Then:
    - m, u or h will make it to go up
    - s make it stop
    - b, or d will make it to go down
    - you can also send a HEX number directly for any weird command you (0x9 for the sun and wind detector for instance)


test: 
-PROG, whem am entry exists to enable to prog more than one blind to an address.




    
*/

#include <EEPROM.h>
#define PORT_TX 5 //5 of PORTD = DigitalPin 5
#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8
#define DEBUG false

long REMOTE = 0x000000;
int EEPROM_ADDRESS = 0;

unsigned int newRollingCode = 101;       //<-- Change it!

int answerNum = 2;
byte frame[7];
byte checksum;
int remoteCount = 0x00;

void BuildFrame(byte *frame, byte button);
void SendCommand(byte *frame, byte sync);


void setup() {
  Serial.begin(115200);
  DDRD |= 1 << PORT_TX; // Pin 5 an output
  PORTD &= !(1 << PORT_TX); // Pin 5 LOW

  Serial.println("20;01;VER=1.0;REV=07;BUILD=0c;");
 
// ----------------------------------------------------------------------------------------------------------------
// INITIAL EEPROM WRITE - START
// ----------------------------------------------------------------------------------------------------------------

  //deleteEEPROMContent();
  //EEPROM.put(0x010, 0x121300); 
  //EEPROM.put(0x016, 0xAF);
  //EEPROM.put(0x0, 0x2);
  //deleteEEPROMContent();
  EEPROM.get(0, remoteCount);

  if (remoteCount == 0xFF){
    EEPROM.put(0, 0x0);
    }
  
  if (DEBUG){Serial.print("RemoteCount: "); Serial.println(remoteCount);}

 
// ----------------------------------------------------------------------------------------------------------------
// INITIAL EEPROM WRITE - STOP
// ----------------------------------------------------------------------------------------------------------------


}

void loop() 
{
  if (Serial.available() > 0) 
  {

// ----------------------------------------------------------------------------------------------------------------
// PARSE SERIAL INPUT - START
// ----------------------------------------------------------------------------------------------------------------

    String readString = Serial.readString();
    readString.toUpperCase(); 
    readString.trim(); 
    if (DEBUG)
    {
      Serial.print("Input String:");
      Serial.println("'"+readString+"'");
    }

    //When serial input contains not a ';'
    if (readString.indexOf(";")== -1)
    { 
      if (DEBUG){Serial.println("Input doesnt contain a ;");}
      if(readString == "DELETE") 
        {
          if (DEBUG)
          {
            Serial.println("Update");
            Serial.print("REMOTE= ");
            Serial.println(REMOTE, HEX);
            Serial.print("EEPROM_ADDRESS=");
            Serial.println(EEPROM_ADDRESS, HEX);
          }

        }
        else if(readString == "DUMP") 
        {
          Serial.println("Show EEPROM");
          //Dump EEPROM
          showEEPROMContent();
        }
        else
        {
          if (DEBUG){Serial.println("Not found - nothing to do");}
        }
    }

    else
    {
        int ind1 = readString.indexOf(';');  //finds location of first ,
        String RTSlinenum = readString.substring(0, ind1);   //captures first data String
        int ind2 = readString.indexOf(';', ind1+1 );   //finds location of second ,
        String RTSrts = readString.substring(ind1+1, ind2);   //captures second data String
        int ind3 = readString.indexOf(';', ind2+1 );
        String RTSaddress = readString.substring(ind2+1, ind3);
        int ind4 = readString.indexOf(';', ind3+1 );
        String RTSoption = readString.substring(ind3+1, ind4); //captures remain part of data after last ,
        int ind5 = readString.indexOf(';', ind4+1 );
        String RTScommand = readString.substring(ind4+1, ind5); //captures remain part of data after last ,
     
        REMOTE = hexToDec(RTSaddress);
        newRollingCode = hexToDec(RTSoption);
          
        
        if (DEBUG)
        {
          Serial.print("RTSlinenum = ");
          Serial.println(RTSlinenum);
          Serial.print("RTSrts = ");
          Serial.println("'"+RTSrts+"'");
          Serial.print("RTSaddress = ");
          Serial.println(RTSaddress);
          Serial.print("RTSaddress(hex) = ");
          Serial.println(REMOTE, HEX);
          Serial.print("RTSoption = ");
          Serial.println(RTSoption);
          Serial.print("RTScommand = ");
          Serial.println("'"+RTScommand+"'");
          Serial.println();
        }
    
    
    
        if (findInEEPROM(REMOTE) == true )
        {
            if(RTScommand == "UP"  || RTScommand == "OFF") 
            {
               if (DEBUG){Serial.println("Moving Up");}
               BuildFrame(frame, HAUT );
            }
            else if(RTScommand == "STOP") 
            {
              if (DEBUG){Serial.println("Stop");}
              BuildFrame(frame, STOP);
            }
            else if(RTScommand == "DOWN" || RTScommand == "ON") 
            {
              if (DEBUG){Serial.println("Moving Down");}
              BuildFrame(frame, BAS);
            }
            else if(RTScommand == "PROG" || RTScommand =="PAIR") 
            {
              //REPROGRAM
              if (DEBUG){Serial.println("Prog");}
              if (DEBUG)
              { 
                Serial.println("Update Programming...");
                Serial.print("REMOTE= ");
                Serial.println(REMOTE, HEX);
                Serial.print("EEPROM_ADDRESS=");
                Serial.println(EEPROM_ADDRESS, HEX);
                
              }
              BuildFrame(frame, PROG);
                          
           }
           else if(RTScommand == "MODEPROG" || RTScommand =="MODEPAIR") 
            {
              //REPROGRAM
              if (DEBUG){Serial.println("Prog");}
              if (DEBUG)
              { 
                Serial.println("Enter Programming Mode...");
                Serial.print("REMOTE= ");
                Serial.println(REMOTE, HEX);
                Serial.print("EEPROM_ADDRESS=");
                Serial.println(EEPROM_ADDRESS, HEX);
                
              }
              //send the normal frames
              BuildFrame(frame, PROG);
              //send additionals frames 
              SendCommand(frame, 2);
              for (int i = 0; i < 10; i++) {
                SendCommand(frame, 7);
              }
                          
           } 
            else 
            {
              if (DEBUG){Serial.println("ERROR INPUT - NOTHING TO DO");}
              //Serial.println("Custom code");
              //BuildFrame(frame, serie);
            }
                      
            RTSprint("OK");
       }
       else
       {
          if(RTScommand == "PROG" || RTScommand =="PAIR") 
          {
            if (DEBUG){Serial.println("Prog");}
            EEPROM_ADDRESS = (remoteCount+1)*16 + 6;
            if (DEBUG){Serial.println(EEPROM_ADDRESS, HEX);}
            EEPROM.put((remoteCount+1)*16,REMOTE);
            EEPROM.put(EEPROM_ADDRESS,newRollingCode);
            remoteCount=remoteCount+1;
            EEPROM.put(0, remoteCount);
            if (DEBUG)
            {
                Serial.println("creating new entry... ");
                Serial.print("REMOTE=");
                Serial.println(REMOTE);
                Serial.print("newRollingCode=");
                Serial.println(newRollingCode);
            }
            BuildFrame(frame, PROG);
            RTSprint("OK");              
          }
          else if(RTScommand == "PING") 
          {
            RTSprint("PONG");
          } 
     
         else
         {
            RTSprint("ERROR");
         }
      }


// ----------------------------------------------------------------------------------------------------------------
// PARSE SERIAL INPUT - END
// ----------------------------------------------------------------------------------------------------------------
    
  if (DEBUG){showEEPROMContent();}
    } // close string enthält ;
  
  } //close Serial.Available

  
} //close void loop


// ----------------------------------------------------------------------------------------------------------------
// SEARCH EEPROM
// ----------------------------------------------------------------------------------------------------------------
bool findInEEPROM(long remoteAddressToFind){
    long readFromEEPROM;
    unsigned int foundInEEPROM;
    int EEPROMaddressMax = EEPROM.length();
    bool found = false;
  
  for (int x = 0; x < EEPROMaddressMax; x = x+16)
  {
    EEPROM.get(x, readFromEEPROM);  
    if (readFromEEPROM == remoteAddressToFind){
      if (DEBUG){
        Serial.print("found:");
        EEPROM.get(x+6, foundInEEPROM);  
        Serial.println(foundInEEPROM);
        Serial.println();
        }
      EEPROM_ADDRESS = x+6;
      found = true;
      break;
    }  
  }
   
  return found;
}

// ----------------------------------------------------------------------------------------------------------------
// Build and Send Frame
// ----------------------------------------------------------------------------------------------------------------
void BuildFrame(byte *frame, byte button) {
  unsigned int code;
  
  EEPROM.get(EEPROM_ADDRESS, code);
  
  frame[0] = 0xA7; // Encryption key. Doesn't matter much
  frame[1] = button << 4;  // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = code >> 8;    // Rolling code (big endian)
  frame[3] = code;         // Rolling code
  frame[4] = REMOTE >> 16; // Remote address
  frame[5] = REMOTE >>  8; // Remote address
  frame[6] = REMOTE;       // Remote address

  if (DEBUG){Serial.print("Frame         : ");}
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant
      if (DEBUG){Serial.print("0"); }    // nibble is a 0.
    }
    if (DEBUG){Serial.print(frame[i], HEX); Serial.print(" ");}
  }

  // Checksum calculation: a XOR of all the nibbles
  checksum = 0;
  for (byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111; // We keep the last 4 bits only


  //Checksum integration
  frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will
  // consider the checksum ok.

  if (DEBUG){Serial.println(""); Serial.print("With checksum : ");}
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) {
      if (DEBUG){Serial.print("0");}
    }
    if (DEBUG){Serial.print(frame[i], HEX); Serial.print(" ");}
  }


  // Obfuscation: a XOR of all the bytes
  for (byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i - 1];
  }

  if (DEBUG){Serial.println(""); Serial.print("Obfuscated    : ");}
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) {
      if (DEBUG){Serial.print("0");}
    }
    if (DEBUG){Serial.print(frame[i], HEX); Serial.print(" ");}
  }
  if (DEBUG){Serial.println("");
  Serial.print("Rolling Code  : "); Serial.println(code);}
  EEPROM.put(EEPROM_ADDRESS, code + 1); //  We store the value of the rolling code in the
  // EEPROM. It should take up to 2 adresses but the
  // Arduino function takes care of it.


  //sending the frame
  SendCommand(frame, 2);
  for (int i = 0; i < 2; i++) {
    SendCommand(frame, 7);
  }

}

// ----------------------------------------------------------------------------------------------------------------
// SendCommand
// ----------------------------------------------------------------------------------------------------------------
void SendCommand(byte *frame, byte sync) {
  if (sync == 2) { // Only with the first frame.
    //Wake-up pulse & Silence
    PORTD |= 1 << PORT_TX;
    delayMicroseconds(9415);
    PORTD &= !(1 << PORT_TX);
    delayMicroseconds(89565);
  }

  // Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    PORTD |= 1 << PORT_TX;
    delayMicroseconds(4 * SYMBOL);
    PORTD &= !(1 << PORT_TX);
    delayMicroseconds(4 * SYMBOL);
  }

  // Software sync
  PORTD |= 1 << PORT_TX;
  delayMicroseconds(4550);
  PORTD &= !(1 << PORT_TX);
  delayMicroseconds(SYMBOL);


  //Data: bits are sent one by one, starting with the MSB.
  for (byte i = 0; i < 56; i++) {
    if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1) {
      PORTD &= !(1 << PORT_TX);
      delayMicroseconds(SYMBOL);
      PORTD ^= 1 << 5;
      delayMicroseconds(SYMBOL);
    }
    else {
      PORTD |= (1 << PORT_TX);
      delayMicroseconds(SYMBOL);
      PORTD ^= 1 << 5;
      delayMicroseconds(SYMBOL);
    }
  }

  PORTD &= !(1 << PORT_TX);
  delayMicroseconds(30415); // Inter-frame silence
}

// ----------------------------------------------------------------------------------------------------------------
// DUMP EEPROM
// ----------------------------------------------------------------------------------------------------------------
void showEEPROMContent()
{
  int address = 0;
  int addressMax = 0;
  byte value;
  int addressL = 0;
  int addressH = 0;


  addressMax = EEPROM.length();
  Serial.println();
  Serial.print("EEProm Size = ");
  Serial.print(addressMax, HEX);
  Serial.print("h Bytes = ");
  Serial.print(addressMax, DEC);
  Serial.println(" Bytes");
  Serial.println();
  Serial.println("       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");


  for (addressH = 0; addressH < remoteCount * 16 + 32; addressH = addressH + 16)
  //for (addressH = 0; addressH < addressMax - 0x300 + 16; addressH = addressH + 16)
  {
    print3digits(addressH);
    Serial.print("  ");
    for (addressL = 0; addressL < 16; addressL++)
    {
      address = addressH + addressL;
      Serial.print(" ");
      value = EEPROM.read(address);
      if (value <16)  {Serial.print("0");}
      Serial.print(value, HEX);

    }
    Serial.println();
  }
  Serial.println();
  Serial.println("Reading finished.");

}

// ----------------------------------------------------------------------------------------------------------------
// DELETE EEPROM
// ----------------------------------------------------------------------------------------------------------------
void deleteEEPROMContent()
{
  int address = 0;
  int addressMax = 0;
  addressMax = EEPROM.length();

  for (address = 0x30; address < addressMax; address++)
      {
        EEPROM.put(address,0xFF);
      }

  Serial.println("EEPROM Delete finished.");

}



// ----------------------------------------------------------------------------------------------------------------
// DUMP EEPROM HELPER
// ----------------------------------------------------------------------------------------------------------------
void print3digits(int number) {
  // Output leading zero
  if (number >= 0 && number < 16) {
    Serial.print('0');
    Serial.print('0');
  }
  if (number >= 16 && number < 256) {
    Serial.print('0');
  }
  Serial.print(number, HEX);
}

// ----------------------------------------------------------------------------------------------------------------
// GENERATE RFLINK ANSWER
// ----------------------------------------------------------------------------------------------------------------
void RTSprint(String RTSinpString){

  Serial.print("20;");
    if (answerNum >= 0 && answerNum < 16) { Serial.print('0'); }
  Serial.print(answerNum,HEX);
  Serial.print(";");
  Serial.print(RTSinpString);
  Serial.println(";");
  
  answerNum= answerNum+1;
  }

long hexToDec(String hexString) {
  
  long decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}
  

