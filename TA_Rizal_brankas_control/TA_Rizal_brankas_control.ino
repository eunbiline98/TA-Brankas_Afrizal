#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define door 5        // door solenoid
#define dryContact 6  // Dry Contact to ESP-32 Cam
#define buzz 11       // buzzer alarm
#define sensDoor 8    // door sensor

int cntAlarm = 0;
int acc = 0;
int stDoor = 0;  // state Door

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

void setup() {
  Serial.begin(9600);
  lcd.init();  // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();

  pinMode(door, OUTPUT);
  pinMode(dryContact, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(sensDoor, INPUT_PULLUP);

  digitalWrite(door, HIGH);
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
  //alarm active function
  if (cntAlarm >= 3) {
    for (int i = 0; i <= 50; i++) {
      // action
      lcd.setCursor(0, 0);
      lcd.print("Brankas Dalam     ");
      lcd.setCursor(0, 1);
      lcd.print("Berbahaya...!!    ");
      // alarm_brankas(750);
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
  }
  if (digitalRead(sensDoor) == 0 && stDoor == 1) {
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
    acc = 0;
  }

  // Monitor Debug
  // Serial.print("status Door = ");
  // Serial.println(stDoor);
  // Serial.print("status sens = ");
  // Serial.println(digitalRead(sensDoor));
  // Serial.print("status acc = ");
  // Serial.println(acc);
  // Serial.print("/////////////////////////");
  // Serial.println();

  delay(50);  //don't need to run this at full speed.
}

// void alarm_brankas(int speed) {
//   digitalWrite(buzz, LOW);
//   static unsigned long time = millis();
//   if (millis() - time >= speed) {
//     time = millis();
//     digitalWrite(buzz, HIGH);
//   }
// }
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
