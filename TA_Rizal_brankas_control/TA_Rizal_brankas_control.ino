#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define door 5        // door solenoid
#define dryContact 6  // Dry Contact to ESP-32 Cam
#define buzz 11       // buzzer alarm
#define sensDoor 8    // door sensor

int cntAlarm = 0;
int cntAlert = 0;
int acc = 0;
int stDoor = 0;  // state Door

unsigned long startMillisPublishDoorSensor;
unsigned long currentMillisPublishDoorSensor;

SoftwareSerial mySerial(3, 2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

byte progressBar[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

void alarmStatus() {
  // if (digitalRead (sensDoor) == 1 && millis() - doorOpenStartTime >= 10000) {
  //   lcd.setCursor(0, 0);
  //   lcd.print("Tutup Pintu       ");
  //   lcd.setCursor(0, 1);
  //   lcd.print("Brankas.....!!    ");
  //   digitalWrite(buzz, HIGH);
  //   lcd.backlight();
  //   delay(500);
  //   digitalWrite(buzz, LOW);
  //   lcd.noBacklight();
  //   delay(500);
  // }
  if (acc == 0) {
    if (digitalRead(sensDoor) == 1) {
      lcd.setCursor(0, 0);
      lcd.print("Pintu Brankas     ");
      lcd.setCursor(0, 1);
      lcd.print("Tutup Kembali !!");
      digitalWrite(buzz, HIGH);
      lcd.backlight();
      delay(500);
      digitalWrite(buzz, LOW);
      lcd.noBacklight();
      delay(500);
    } else if (digitalRead(sensDoor) == 1) {
      digitalWrite(buzz, LOW);
    }
  }
}


void countdownTimer(int seconds) {
  if (seconds > 0) {
    Serial.println(seconds);
    delay(1000);
    seconds -= 1;
  }
  if (digitalRead(sensDoor) == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Tutup Pintu       ");
    lcd.setCursor(0, 1);
    lcd.print("Brankas.....!!    ");
    digitalWrite(buzz, HIGH);
    lcd.backlight();
    delay(500);
    digitalWrite(buzz, LOW);
    lcd.noBacklight();
    delay(500);
  } else if (digitalRead(sensDoor) == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Tutup Pintu       ");
    lcd.setCursor(0, 1);
    lcd.print("Brankas.....!!    ");
    digitalWrite(buzz, HIGH);
    lcd.backlight();
    delay(500);
    digitalWrite(buzz, LOW);
    lcd.noBacklight();
    delay(500);
  }
  Serial.println("Countdown selesai");
}

void setup() {
  Serial.begin(9600);
  lcd.init();  // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();

  pinMode(door, OUTPUT);
  pinMode(dryContact, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(sensDoor, INPUT_PULLUP);

  digitalWrite(door, LOW);
  digitalWrite(dryContact, HIGH);

  lcd.createChar(0, progressBar);
  lcd.setCursor(0, 0);
  lcd.print(" System Brankas");
  lcd.setCursor(5, 1);
  lcd.print("Ver 1.1");
  delay(1000);
  lcd.clear();

  for (int i = 0; i <= 16; i++) {
    lcd.setCursor(0, 0);
    lcd.print("Please Wait...!!!       ");
    lcd.setCursor(i, 1);
    lcd.write(byte(0));
    delay(100);
  }
  lcd.clear();
  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    lcd.setCursor(0, 0);
    lcd.print("  FingerPrint ");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Connected");
    delay(3000);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("  FingerPrint ");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Error...!");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Check Sensor");
    lcd.setCursor(0, 1);
    lcd.print("Please...!");

    while (1) {
      delay(1);
    }
  }
}

void loop()  // run over and over again
{
  lcd.backlight();
  doorlockAccess();
  alarmStatus();
  //alarm active function
  if (cntAlarm >= 3) {
    for (int i = 0; i <= 10; i++) {
      // action
      lcd.setCursor(0, 0);
      lcd.print("Brankas Dalam     ");
      lcd.setCursor(0, 1);
      lcd.print("Berbahaya...!!    ");
      digitalWrite(buzz, HIGH);
      lcd.backlight();
      delay(500);
      digitalWrite(buzz, LOW);
      lcd.noBacklight();
      delay(500);
    }
    cntAlarm = 0;
  } else {
    digitalWrite(buzz, LOW);
  }

  if (acc == 1 && digitalRead(sensDoor) == 1 && stDoor == 0) {
    stDoor = 1;
  } else if (acc == 1 && digitalRead(sensDoor) == 1 && stDoor == 1) {
    currentMillisPublishDoorSensor = millis();
    if (currentMillisPublishDoorSensor - startMillisPublishDoorSensor >= 1000) {
      cntAlert++;
      startMillisPublishDoorSensor = millis();
    }
  } else if (digitalRead(sensDoor) == 0 && stDoor == 1) {
    digitalWrite(dryContact, LOW);
    delay(3000);
    digitalWrite(dryContact, HIGH);
    Serial.println("Pintu Terkunci");
    lcd.setCursor(0, 0);
    lcd.print("Brankas Terkunci  ");
    lcd.setCursor(0, 1);
    lcd.print("                  ");
    delay(2000);
    digitalWrite(door, HIGH);
    stDoor = 0;
    cntAlert = 0;
    acc = 0;
  }

  if (cntAlert >= 10) {
    lcd.setCursor(0, 0);
    lcd.print("Tutup Pintu       ");
    lcd.setCursor(0, 1);
    lcd.print("Brankas.....!!    ");
    digitalWrite(buzz, HIGH);
    lcd.backlight();
    delay(500);
    digitalWrite(buzz, LOW);
    lcd.noBacklight();
    delay(500);
  }

  // Monitor Debug
  Serial.print("status Door = ");
  Serial.println(stDoor);
  Serial.print("status sens = ");
  Serial.println(digitalRead(sensDoor));
  Serial.print("status acc = ");
  Serial.println(acc);
  Serial.print("status alert = ");
  Serial.println(cntAlert);
  Serial.print("/////////////////////////");
  Serial.println();

  delay(200);  //don't need to run this at full speed.
}

// void
// Main Program
int doorlockAccess() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    lcd.setCursor(0, 0);
    lcd.print(" Silahkan Scan  ");
    lcd.setCursor(0, 1);
    lcd.print("   Jari Anda    ");
    return -1;
  }
  // Akses terdaftar dan gagal baca
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    digitalWrite(buzz, HIGH);
    delay(300);
    digitalWrite(buzz, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Silahkan Scan  ");
    lcd.setCursor(0, 1);
    lcd.print("Kembali...!!    ");
    delay(3000);
    lcd.clear();
    return -1;
  }
  // Akses tidak terdaftar
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    digitalWrite(buzz, HIGH);
    delay(300);
    digitalWrite(buzz, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   Akses Anda   ");
    lcd.setCursor(0, 1);
    lcd.print("Tidak Terdaftar!");
    delay(3000);
    lcd.clear();
    cntAlarm++;
    return -1;
  }
  // Akses terdaftar dan success
  digitalWrite(buzz, HIGH);
  delay(300);
  digitalWrite(buzz, LOW);
  lcd.clear();
  Serial.println("Pintu Terbuka");
  lcd.setCursor(0, 0);
  lcd.print("Brankas Terbuka   ");
  lcd.setCursor(0, 1);
  lcd.print("Hello...          ");
  digitalWrite(door, LOW);
  digitalWrite(dryContact, LOW);
  delay(3000);
  digitalWrite(dryContact, HIGH);
  lcd.clear();
  cntAlarm = 0;
  acc = 1;
  return finger.fingerID;
}
