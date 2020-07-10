#include <SPI.h>
#include <MFRC522.h>

const int rstPin = 3;                                                     // Reset pin in RFID
const int ssPin = 10;                                                     // SDA pin in RFID
const int sensorPin = 2;                                                  // PIR sensor pin
const int buttonPin = 8; 
const int buzzerPin = 5;                                                  // Active buzzer pin


MFRC522 mfrc522(ssPin, rstPin);                                           // Create MFRC522 instance
MFRC522::MIFARE_Key key;                                                  // Create a MIFARE_Key struct named 'key', which will hold the card information

bool securitySystemEnabled = false;                                       // System starts off disabled

String masterID = "6945BFA3";                                             // UID of the correct PICC
String userID = "";

void setup() {
  Serial.begin(9600);
  SPI.begin();                                                            // Init SPI bus
  mfrc522.PCD_Init();                                                     // Init MFRC522 
  pinMode(buttonPin, INPUT);
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  for (byte i = 0; i < 6; i++) {                                          //MIFIRE card uses factory settings
    key.keyByte[i] = 0xFF;
  }
  

}

void loop() {
     if(!securitySystemEnabled)                                           // Waits for a button press to enable the security system
      {
        if(digitalRead(buttonPin) == HIGH )
        {
          securitySystemEnabled = true;               
          Serial.println("System Enabled");
          delay(10000);                                                   // Gives a delay of 10 seconds for the user to get out of range
        }
      }
      else if(digitalRead(sensorPin) == HIGH)                             // Checks if the PIR sensor detects motion
      {
        Serial.println("Motion detected");
        long timeAtStartOfMovement = millis();
        long chipReadingInterval = 30000;                                 // Interval during which the PRID cards can be detected
        while(millis()- timeAtStartOfMovement <= chipReadingInterval)
        {
          while(getID())                                                  // Checks for a PICC
          {     
             if (userID == masterID)                                      // Checks if the UID received is correct
             {
              Serial.println("Correct tag!");
              allowSecuritySystemDeactivation(30000); 
              return;
             }
             else
             {        
              Serial.println("Incorrect tag, please try again.");
              incorrectIDBuzz(700, 1000);                   
             }     
           }    
         }
       soundAlarm(60000, 800, 500, 1000);                                // Turns on the alarm if a correct PICC hasn't been detected
       }  
}

void soundAlarm(long totalDuration, int buzzDuration, int delayBetweenBuzzes, int buzzFrequency)  // Activates the active buzzer
{
  Serial.println("Intruder detected!");
  Serial.println("Sounding alarm!");
  for(int i = 0; i < totalDuration/buzzDuration; i++)                    // Repeats the buzzer for the desired duration
  {
      tone(buzzerPin, buzzFrequency);
      delay(delayBetweenBuzzes);     
      noTone(buzzerPin);
      if(getID()) // Checks for a PICC
      {
        if(userID == masterID)                                           // Checks if the UID received is correct
        {
          Serial.println("Correct tag!");
          allowSecuritySystemDeactivation(15000);
          return;
        }
        else
        {
          incorrectIDBuzz(buzzDuration, buzzFrequency + 200);
          Serial.println("Incorrect tag, please try again.");
        }
      }
      delay(delayBetweenBuzzes);
  }
}

void incorrectIDBuzz(int buzzDuration, int buzzFrequency)
{
  tone(buzzerPin,buzzFrequency);
  delay(buzzDuration);
  noTone(buzzerPin);
}

void allowSecuritySystemDeactivation(long waitTime)                     // Allows the security system to be deactivated if the button is pressed
{
  long startTime = millis();  
  while(millis() - startTime <= waitTime) 
    {
       if(digitalRead(buttonPin) == HIGH)
       {       
         securitySystemEnabled = false;
         Serial.println("System Disabled");
         delay(10000);
         return;
       }
    }
       Serial.println("System hasn't been disabled");
       return;
}

boolean getID()                                                       // Reads a PICC and stores its UID in userID
{
  if ( ! mfrc522.PICC_IsNewCardPresent()) {                           //If a new PICC placed to RFID reader continue
    return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {                             //Since a PICC placed get Serial and continue
    return false;
  }
  userID = "";
  for ( uint8_t i = 0; i < 4; i++)                                    // The MIFARE PICCs that we use have 4 byte UID
  { 
    userID.concat(String(mfrc522.uid.uidByte[i], HEX));               // Adds the 4 bytes in a single String variable
  }
  userID.toUpperCase();
  mfrc522.PICC_HaltA();                                               // Stop reading
  return true;
}
