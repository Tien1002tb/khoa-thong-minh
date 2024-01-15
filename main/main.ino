#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class

MFRC522::MIFARE_Key mkey;
char d, d1, d2, d3, d10, d11, d12, d13;
int c, cpa, tag;
bool cp, np, ok, m;
// Init array that will store new NUID
byte nuidPICC[4];
Servo myservo;
int pos = 0;

//-------keypad setting-----------------
const byte ROWS = 4;  //four rows
const byte COLS = 3;  //three columns
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte rowPins[ROWS] = { 2, 3, 4, 5 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 6, 7, 8 };     //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//-------------lcd function declaration---------------------



void setup() {
  Serial.begin(9600);
  myservo.attach(14);


  lcd.begin();
  lcd.backlight();
  lcd.print("SCAN YOUR ID TAG");
  lcd.setCursor(0, 1);


  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    mkey.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  //--set def pass
  if (EEPROM.read(0) > 1 | EEPROM.read(0) < 1) {
    EEPROM.update(0, 1);
    EEPROM.update(1, '1');
    EEPROM.update(2, '2');
    EEPROM.update(3, '3');
    EEPROM.update(4, '4');
  }
  d = EEPROM.read(1);
  d1 = EEPROM.read(2);
  d2 = EEPROM.read(3);
  d3 = EEPROM.read(4);
}

void loop() {

  //--------------------------keypad sec code----------------------
  char k = keypad.getKey();

  if (k) {
    c++;
    Serial.println(k);

    lcd.print("*");
    if (c == 1) {
      d10 = k;
    }
    if (c == 2) {
      d11 = k;
    }
    if (c == 3) {
      d12 = k;
    }
    if (c == 4) {
      d13 = k;
    }
  }
  if (k == '#') {
    m = c = cp = np = tag = 0;
    lcd.clear();
    lcd.print("SCAN YOUR ID TAG");
  }



  if (c == 4 & np == 0) {
    c = 0;
    if (d10 == '*' & d11 == '0' & d12 == '0') {
      m = 1;
      lcd.clear();
      lcd.print("ENTER PASSWORD   ");
      switch (d13) {
        case '0':
          cp = 1;

          break;
        case '1':

          tag = 1;
          break;
        case '2':
          tag = 2;
          break;
        case '3':
          tag = 3;
          break;
        case '4':
          tag = 4;
          break;
        case '5':
          tag = 5;
          break;
      }
      d10 = d11 = d12 = d13 = 0;
    }
    if (d == d10 & d1 == d11 & d2 == d12 & d3 == d13) {
      if (cp == 0 & m == 0) {
        lcd.setCursor(0, 1);
        lcd.print("PASSWORD ACCEPTED");


        for (pos = 0; pos <= 180; pos++) {  // Quay từ 0 đến 180 độ
          myservo.write(pos);
          delay(5);
        }
        delay(4000);
        for (pos = 180; pos >= 0; pos--) {  // Quay từ 180 về 0 độ
          myservo.write(pos);
          delay(5);
        }


        d10 = d11 = d12 = d13 = 0;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
      }
      if (cp == 1 & tag == 0) {
        lcd.clear();
        lcd.print("NEW PASSWORD");
        lcd.setCursor(0, 1);
        np = 1;
      }
      if (tag > 0) {

        lcd.clear();
        lcd.print("SCAN ID TAG # ");
        lcd.print(tag);
        lcd.setCursor(0, 1);
      }
      m = 0;
    } else {
      if (!m) {
        lcd.setCursor(0, 1);
        lcd.print("WRONG PASSWORD");
        delay(3000);
      }


      d10 = d11 = d12 = d13 = 0;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
    }
  }





  if (c == 4 & np == 1) {
    m = 0;
    d = d10;
    d1 = d11;
    d2 = d12;
    d3 = d13;
    EEPROM.update(1, d);
    EEPROM.update(2, d1);
    EEPROM.update(3, d2);
    EEPROM.update(4, d3);
    np = 0;
    cp = 0;
    c = 0;
    d10 = d11 = d12 = d13 = 0;
    lcd.clear();
    lcd.print("SCAN YOUR ID TAG");
    lcd.setCursor(0, 1);
  }

  //---rfid----------------------
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  if (tag > 0) {
    switch (tag) {
      case 1:
        EEPROM.update(5, rfid.uid.uidByte[0]);
        EEPROM.update(6, rfid.uid.uidByte[1]);
        EEPROM.update(7, rfid.uid.uidByte[2]);
        EEPROM.update(8, rfid.uid.uidByte[3]);
        break;
      case 2:
        EEPROM.update(9, rfid.uid.uidByte[0]);
        EEPROM.update(10, rfid.uid.uidByte[1]);
        EEPROM.update(11, rfid.uid.uidByte[2]);
        EEPROM.update(12, rfid.uid.uidByte[3]);
        break;
      case 3:
        EEPROM.update(13, rfid.uid.uidByte[0]);
        EEPROM.update(14, rfid.uid.uidByte[1]);
        EEPROM.update(15, rfid.uid.uidByte[2]);
        EEPROM.update(16, rfid.uid.uidByte[3]);
        break;
      case 4:
        EEPROM.update(17, rfid.uid.uidByte[0]);
        EEPROM.update(18, rfid.uid.uidByte[1]);
        EEPROM.update(19, rfid.uid.uidByte[2]);
        EEPROM.update(20, rfid.uid.uidByte[3]);
        break;
      case 5:
        EEPROM.update(21, rfid.uid.uidByte[0]);
        EEPROM.update(22, rfid.uid.uidByte[1]);
        EEPROM.update(23, rfid.uid.uidByte[2]);
        EEPROM.update(24, rfid.uid.uidByte[3]);
        break;
    }
    tag = 0;
    d10 = d11 = d12 = d13 = 0;
    lcd.setCursor(0, 1);
    lcd.print("  TAG ID SAVED  ");

    delay(2000);
    lcd.clear();
    lcd.print("SCAN YOUR ID TAG");

    cp = 0;
    c = 0;
  } else
    for (int i = 1; i < 6; i++) {
      if (rfid.uid.uidByte[0] == EEPROM.read((i * 4) + 1) & rfid.uid.uidByte[1] == EEPROM.read((i * 4) + 2) & rfid.uid.uidByte[2] == EEPROM.read((i * 4) + 3) & rfid.uid.uidByte[3] == EEPROM.read((i * 4) + 4)) ok = 1;
    }

  if (ok == 1) {
    ok = 0;
    Serial.println(F("A new card has been detected."));
    lcd.setCursor(0, 1);
    lcd.print("ID TAG ACCEPTED ");



    for (pos = 0; pos <= 180; pos++) {  // Quay từ 0 đến 180 độ
      myservo.write(pos);
      delay(5);
    }

    delay(2000);

    lcd.setCursor(0, 1);
    lcd.print("                ");
    for (pos = 180; pos >= 0; pos--) {  // Quay từ 180 về 0 độ
      myservo.write(pos);
      delay(5);
    }
    return;
  } else {
    lcd.setCursor(0, 1);
    lcd.print(" ACCESS DENIED ");

    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }




  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
//------------------loop end function start---------------------
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
