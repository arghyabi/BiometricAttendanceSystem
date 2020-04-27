#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>

RTC_DS1307 rtc;

const int load_button = 4, return_button = 5, led_blue = 10, led_green = 11, led_red = 12, buzzer = 13, EEPROM_erase_button = A2, mode_switch = A3;

LiquidCrystal_I2C lcd(0x27, 16, 2);


SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int check_return = 0, total_id = 0;
const int dly = 20;
int Mode = HIGH;
int buttonState, buttonState2;
int lastButtonState = HIGH, lastButtonState2 = HIGH;
unsigned long lastDebounceTime = 0, lastDebounceTime2 = 0;
unsigned long debounceDelay = 50, debounceDelay2 = 2000;

uint8_t id;

void setup() {
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  pinMode(buzzer, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(mode_switch, INPUT);
  pinMode(load_button, INPUT);
  pinMode(return_button, INPUT);
  pinMode(EEPROM_erase_button, INPUT);

  int init_delay = 500;
  lcd.setCursor(0, 1);
  lcd.print("By Arghya Biswas");

  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, 0);
    lcd.print("Initializing    ");
    delay(init_delay);

    lcd.setCursor(0, 0);
    lcd.print("Initializing.   ");
    delay(init_delay);

    lcd.setCursor(0, 0);
    lcd.print("Initializing..  ");
    delay(init_delay);

    lcd.setCursor(0, 0);
    lcd.print("Initializing... ");
    delay(init_delay);
  }


  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  finger.begin(57600);

  if (finger.verifyPassword()) {
    delay(dly);
  } else {
    while (1) {
      delay(1);
      lcd.setCursor(0, 0);
      lcd.print(" SENSOR MISSING ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
  }

  finger.getTemplateCount();
  total_id = finger.templateCount;
}



void loop()                     // run over and over again
{
  DateTime now = rtc.now();
  int T_hour = now.hour();
  int T_minute = now.minute();
  int T_second = now.second();

  if (digitalRead(mode_switch) == HIGH) {
    Mode = 0;
  } else {
    Mode = 1;
  }


  if (digitalRead(EEPROM_erase_button) == HIGH) {
    for (int p = 1; p <= total_id*4; p++) {
      EEPROM.write(p, 0);
    }
    tone(buzzer, 440, 60);
  }


  int reading = digitalRead(return_button);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        String all_data = "";
        String all_time = "";
        for (int p = 1; p <= total_id * 4; p = p + 4) {
          if (EEPROM.read(p) == 1) {
            all_data = all_data + "," + String(1);
            all_time = all_time + "," + EEPROM.read(p + 1) + ":" + EEPROM.read(p + 2) + ":" + EEPROM.read(p + 3);
          }
          else {
            all_data = all_data + ",0";
            all_time = all_time + ",0:0:0";
          }
        }
        Serial.println(all_data.substring(1) + "*" + all_time.substring(1));
      }
    }
  }
  lastButtonState = reading;



  int reading2 = digitalRead(load_button);
  if (reading2 != lastButtonState2) {
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay2) {
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
      if (buttonState2 == LOW) {
        finger.emptyDatabase();
        lcd.setCursor(0, 0);
        lcd.print("Database deleted");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        delay(2000);
        total_id = 0;
      }
    }
  }
  lastButtonState2 = reading2;




  if (Mode == HIGH) {
    digitalWrite(led_blue, HIGH); //blue led on
    digitalWrite(led_red, LOW);  //red led off
    if (check_return == 0 || check_return == -1) {
      lcd.setCursor(0, 0);
      lcd.print("_Check-in Mode__");
      lcd.setCursor(0, 1);
      lcd.print("Data=");
      lcd.setCursor(5, 1);
      lcd.print(total_id);
      lcd.print(" ");
      lcd.print(T_hour);
      lcd.print(":");
      lcd.print(T_minute);
      lcd.print(":");
      lcd.print(T_second);
      lcd.print("   ");
    }
    else if (check_return != 0 && check_return != -1) { //when detect
      digitalWrite(led_blue, LOW); // blue off
      digitalWrite(led_red, LOW);  //red off
      tone(buzzer, 440, 600);
      delay(2000);
      digitalWrite(led_red, LOW);
      digitalWrite(led_blue, HIGH);
      digitalWrite(led_green, LOW);
      lcd.setCursor(0, 0);
      lcd.print("_Check-in Mode__");
      lcd.setCursor(0, 1);
      lcd.print("Data=");
      lcd.setCursor(5, 1);
      lcd.print(total_id);
      lcd.print(" ");
      lcd.print(T_hour);
      lcd.print(":");
      lcd.print(T_minute);
      lcd.print(":");
      lcd.print(T_second);
      lcd.print("   ");
      //Serial.println("s2 " + String(Mode));
    }
    check_return = getFingerprintIDez();
  }
  else if (Mode == LOW) {
    digitalWrite(led_blue, LOW); //blue off
    digitalWrite(led_red, HIGH); // red on
    lcd.setCursor(0, 0);
    lcd.print("_Data Load Mode_");
    lcd.setCursor(0, 1);
    lcd.print("Press Load Btn  ");
    //Serial.println("s3 " + String(Mode) + " =");

    id = total_id + 1;

    //Serial.print("Enrolling ID #");
    //Serial.println(id);
    //Serial.println("s3 " + String(Mode) + " ==");
    if (digitalRead(load_button) == LOW) {
      lcd.setCursor(0, 0);
      lcd.print("Put the finger  ");
      lcd.setCursor(0, 1);
      lcd.print("ID # ");
      lcd.print(id);
      lcd.print("                ");
      int enroll_return = getFingerprintEnroll();
      if (enroll_return == 0) {
        total_id = total_id + 1;
        lcd.setCursor(0, 0);
        lcd.print("Load Successfull");
        lcd.setCursor(0, 1);
        lcd.print("Loaded id # ");
        lcd.setCursor(12, 1);
        lcd.print(id);

      }
      else {
        lcd.setCursor(0, 0);
        lcd.print("Load Failed     ");
      }
      //Serial.println("Arghya The entroll retun is : " + String(enroll_return));
      delay(2000);
    }

    //Serial.println("s3 " + String(Mode) + " ===");
  }
  //delay(200);            //don't ned to run this at full speed.
}





uint8_t getFingerprintEnroll() {

  int p = -1;
  //Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        delay(dly);
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.println(".");
        delay(dly);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        delay(dly);
        break;
      case FINGERPRINT_IMAGEFAIL:
        //Serial.println("Imaging error");
        delay(dly);
        break;
      default:
        //Serial.println("Unknown error");
        delay(dly);
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      delay(dly);
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      delay(dly);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      delay(dly);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      delay(dly);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      delay(dly);
      return p;
    default:
      //Serial.println("Unknown error");
      delay(dly);
      return p;
  }
  lcd.setCursor(0, 0);
  lcd.print("Remove finger   ");
  //Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  //Serial.print("ID "); Serial.println(id);
  p = -1;
  lcd.setCursor(0, 0);
  lcd.print("Put once again  ");
  //Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        delay(dly);
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.print(".");
        delay(dly);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        delay(dly);
        break;
      case FINGERPRINT_IMAGEFAIL:
        //Serial.println("Imaging error");
        delay(dly);
        break;
      default:
        //Serial.println("Unknown error");
        delay(dly);
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      delay(dly);
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      delay(dly);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      delay(dly);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      delay(dly);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      delay(dly);
      return p;
    default:
      //Serial.println("Unknown error");
      delay(dly);
      return p;
  }

  // OK converted!
  //Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Prints matched!");
    delay(dly);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    delay(dly);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    //Serial.println("Fingerprints did not match");
    delay(dly);
    return p;
  } else {
    //Serial.println("Unknown error");
    delay(dly);
    return p;
  }

  //Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    //Serial.println("Stored!");
    delay(dly);
    return 0;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    delay(dly);
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    //Serial.println("Could not store in that location");
    delay(dly);
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    //Serial.println("Error writing to flash");
    delay(dly);
    return p;
  } else {
    //Serial.println("Unknown error");
    delay(dly);
    return p;
  }
}





int getFingerprintIDez() {
  DateTime now = rtc.now();

  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;


  int id = finger.fingerID;
  int temp_id = (id - 1) * 4 + 1;
  EEPROM.write(temp_id++, 1);
  EEPROM.write(temp_id++, now.hour());
  EEPROM.write(temp_id++, now.minute());
  EEPROM.write(temp_id++, now.second());


  digitalWrite(led_green, HIGH);  // green led on
  digitalWrite(led_blue, LOW);   // blue off
  digitalWrite(led_red, LOW);  // red off
  lcd.setCursor(0, 0);
  lcd.print("Finger Detected ");
  lcd.setCursor(0, 1);
  lcd.print("ID ");
  lcd.print(id);
  lcd.print("            ");

  return finger.fingerID;
}
