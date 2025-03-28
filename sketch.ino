#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// Initialize RTC and LCD
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change address if needed

// Pin definitions
const int buzzerPin = 8;
const int setTimeButton = 2;    // Button to set the hour
const int setMinuteButton = 3;  // Button to set the minute
const int setAlarmButton = 4;   // Button to set the alarm time
const int alarmToggleButton = 5;// Button to toggle alarm on/off
const int snoozeButton = 6;     // Button to snooze the alarm
const int ledPin = 13;          // Optional LED

// Variables
DateTime now;
DateTime alarmTime;
bool alarmSet = false;
bool alarmActive = false;
bool alarmEnabled = true;       // Alarm enabled state
String alarmMessage = "Wake Up!";

// Button debouncing variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(9600);

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.backlight();

  // Set pin modes
  pinMode(buzzerPin, OUTPUT);
  pinMode(setTimeButton, INPUT_PULLUP);
  pinMode(setMinuteButton, INPUT_PULLUP);
  pinMode(setAlarmButton, INPUT_PULLUP);
  pinMode(alarmToggleButton, INPUT_PULLUP);
  pinMode(snoozeButton, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  // Set initial alarm time
  alarmTime = DateTime(2023, 10, 1, 7, 0, 0);
}

void loop() {
  now = rtc.now();
  displayDateTime();
  checkButtons();
  
  if (alarmEnabled && alarmSet && !alarmActive && 
      now.hour() == alarmTime.hour() && 
      now.minute() == alarmTime.minute()) {
    triggerAlarm();
  }

  delay(50);
}

void displayDateTime() {
  // Line 1: Date
  lcd.setCursor(0, 0);
  lcd.print(now.year());
  lcd.print("-");
  if (now.month() < 10) lcd.print("0");
  lcd.print(now.month());
  lcd.print("-");
  if (now.day() < 10) lcd.print("0");
  lcd.print(now.day());
  
  // Line 2: Time and alarm status
  lcd.setCursor(0, 1);
  lcd.print(now.hour() < 10 ? "0" : "");
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute() < 10 ? "0" : "");
  lcd.print(now.minute());
  lcd.print(" ");
  lcd.print(alarmEnabled ? (alarmSet ? "A:ON" : "A:OFF") : "A:DIS");
}

void checkButtons() {
  // Debounce check
  if ((millis() - lastDebounceTime) < debounceDelay) return;
  lastDebounceTime = millis();

  if (digitalRead(setTimeButton) == LOW) {
    setTime();
    while (digitalRead(setTimeButton) == LOW); // Wait for release
  }
  
  if (digitalRead(setMinuteButton) == LOW) {
    setMinute();
    while (digitalRead(setMinuteButton) == LOW);
  }
  
  if (digitalRead(setAlarmButton) == LOW) {
    setAlarmTime();
    while (digitalRead(setAlarmButton) == LOW);
  }
  
  if (digitalRead(alarmToggleButton) == LOW) {
    toggleAlarm();
    while (digitalRead(alarmToggleButton) == LOW);
  }
  
  if (digitalRead(snoozeButton) == LOW && alarmActive) {
    snoozeAlarm();
    while (digitalRead(snoozeButton) == LOW);
  }
}

void toggleAlarm() {
  alarmEnabled = !alarmEnabled;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm ");
  lcd.print(alarmEnabled ? "ENABLED" : "DISABLED");
  delay(1000);
}

void setTime() {
  int newHour = now.hour();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Hour:");
  
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print(newHour < 10 ? "0" : "");
    lcd.print(newHour);
    lcd.print(":00");
    
    if (digitalRead(setTimeButton) == LOW) {
      delay(200);
      if (digitalRead(setTimeButton) == LOW) {
        newHour = (newHour + 1) % 24;
      }
    }
    
    if (digitalRead(setMinuteButton) == LOW) {
      delay(200);
      break;
    }
  }
  
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), newHour, now.minute(), 0));
  lcd.clear();
  lcd.print("Time Updated!");
  delay(1000);
}

void setMinute() {
  int newMinute = now.minute();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Minute:");
  
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("00:");
    lcd.print(newMinute < 10 ? "0" : "");
    lcd.print(newMinute);
    
    if (digitalRead(setMinuteButton) == LOW) {
      delay(200);
      if (digitalRead(setMinuteButton) == LOW) {
        newMinute = (newMinute + 1) % 60;
      }
    }
    
    if (digitalRead(setTimeButton) == LOW) {
      delay(200);
      break;
    }
  }
  
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), newMinute, 0));
  lcd.clear();
  lcd.print("Time Updated!");
  delay(1000);
}

void setAlarmTime() {
  int newHour = alarmTime.hour();
  int newMinute = alarmTime.minute();
  bool settingHour = true;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Alarm Time");
  
  while (true) {
    lcd.setCursor(0, 1);
    if (settingHour) {
      lcd.print("Hour:");
      lcd.print(newHour < 10 ? "0" : "");
      lcd.print(newHour);
    } else {
      lcd.print("Min:");
      lcd.print(newMinute < 10 ? "0" : "");
      lcd.print(newMinute);
    }
    
    if (digitalRead(setAlarmButton) == LOW) {
      delay(200);
      if (digitalRead(setAlarmButton) == LOW) {
        if (settingHour) {
          newHour = (newHour + 1) % 24;
        } else {
          newMinute = (newMinute + 1) % 60;
        }
      }
    }
    
    if (digitalRead(alarmToggleButton) == LOW) {
      delay(200);
      settingHour = !settingHour;
    }
    
    if (digitalRead(snoozeButton) == LOW) {
      delay(200);
      break;
    }
  }
  
  alarmTime = DateTime(now.year(), now.month(), now.day(), newHour, newMinute, 0);
  alarmSet = true;
  lcd.clear();
  lcd.print("Alarm Set!");
  delay(1000);
}

void triggerAlarm() {
  alarmActive = true;
  digitalWrite(ledPin, HIGH);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(alarmMessage);
  lcd.setCursor(0, 1);
  lcd.print("Press to snooze");
  
  while (alarmActive) {
    tone(buzzerPin, 1000, 500);
    delay(600);
    
    if (digitalRead(snoozeButton) == LOW) {
      snoozeAlarm();
    }
    
    if (digitalRead(alarmToggleButton) == LOW) {
      alarmActive = false;
      digitalWrite(ledPin, LOW);
      noTone(buzzerPin);
      lcd.clear();
      lcd.print("Alarm Stopped");
      delay(1000);
    }
  }
}

void snoozeAlarm() {
  alarmActive = false;
  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);
  
  // Snooze for 5 minutes
  alarmTime = alarmTime + TimeSpan(0, 0, 5, 0);
  lcd.clear();
  lcd.print("Snoozed 5 min");
  delay(1000);
}