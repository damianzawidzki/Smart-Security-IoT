#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

#define TRIG_PIN 7
#define ECHO_PIN A3

#define SERVO_PIN 10
#define GREEN_LED 11
#define RED_LED 12
#define BUZZER 13

#define ESP_MOVE_SIGNAL A1
#define ESP_ALARM_SIGNAL A2
#define ESP_COMMAND_PIN A0

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo doorLock;

char keys[4][3] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[4] = {9, 8, 6, 5};
byte colPins[3] = {4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 3);

String enteredPin = "";
String correctPin = "1234";

bool alarmOn = true;
bool pinModeActive = false;
bool movementScreenActive = false;

long currentDistance = 999;

unsigned long alarmStartTime = 0;
const unsigned long warmupTime = 3000;

unsigned long lastSensorCheck = 0;
const unsigned long sensorCheckInterval = 300;

unsigned long lastLcdUpdate = 0;
const unsigned long lcdUpdateInterval = 900;

unsigned long lastMotionTime = 0;
unsigned long motionCooldown = 60000;

unsigned long movementScreenStart = 0;
const unsigned long movementScreenTime = 3000;

unsigned long espMoveSignalStart = 0;
const unsigned long espMoveSignalTime = 1500;

unsigned long lastDigitTime = 0;
bool showingNumbers = false;
const unsigned long showNumbersTime = 1000;

const int detectionDistance = 120;

// ESP command 
int pulseCount = 0;
bool lastCommandState = LOW;
unsigned long lastPulseTime = 0;
const unsigned long commandTimeout = 1200;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(ESP_MOVE_SIGNAL, OUTPUT);
  pinMode(ESP_ALARM_SIGNAL, OUTPUT);

  pinMode(ESP_COMMAND_PIN, INPUT);

  digitalWrite(ESP_MOVE_SIGNAL, LOW);
  digitalWrite(ESP_ALARM_SIGNAL, HIGH);

  doorLock.attach(SERVO_PIN);
  doorLock.write(0);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  alarmStartTime = millis();

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, LOW);

  showStartingScreen();
  beepArmSound();

  delay(1000);
  showArmedScreen();
}

void loop() {
  readEspCommand();

  char key = keypad.getKey();

  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    handleKey(key);
  }

  hideNumbersAfterDelay();
  checkMovement();
  updateArmedScreen();
  updateMovementScreen();
  updateEspMoveSignal();
}

void readEspCommand() {
  bool commandState = digitalRead(ESP_COMMAND_PIN);

  if (commandState == HIGH && lastCommandState == LOW) {
    pulseCount++;
    lastPulseTime = millis();

    Serial.print("ESP pulse received: ");
    Serial.println(pulseCount);
  }

  lastCommandState = commandState;

  if (pulseCount > 0 && millis() - lastPulseTime > commandTimeout) {
    if (pulseCount == 1) {
      motionCooldown = 10000;
      showMovementTime("10 seconds");
    } 
    else if (pulseCount == 2) {
      motionCooldown = 30000;
      showMovementTime("30 seconds");
    } 
    else if (pulseCount == 3) {
      motionCooldown = 60000;
      showMovementTime("1 minute");
    } 
    else if (pulseCount == 4) {
      motionCooldown = 300000;
      showMovementTime("5 minutes");
    }

    pulseCount = 0;
  }
}

void showMovementTime(String text) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Movement time");
  lcd.setCursor(0, 1);
  lcd.print(text);

  beepShort();

  delay(1500);

  if (pinModeActive) {
    startPinMode();
  } 
  else if (alarmOn) {
    showArmedScreen();
  } 
  else {
    showDisarmedScreen();
  }
}

void handleKey(char key) {
  if (key == '*') {
    armSystem();
    return;
  }

  if (key == '#') {
    if (!pinModeActive) {
      startPinMode();
    } else {
      checkPin();
    }
    return;
  }

  if (pinModeActive && key >= '0' && key <= '9') {
    if (enteredPin.length() < 4) {
      enteredPin += key;
      lastDigitTime = millis();
      showingNumbers = true;
      showPinNumbers();
    }
  }
}

void startPinMode() {
  pinModeActive = true;
  movementScreenActive = false;
  enteredPin = "";
  showingNumbers = false;

  digitalWrite(ESP_MOVE_SIGNAL, LOW);
  digitalWrite(BUZZER, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER PIN");
  lcd.setCursor(0, 1);
  lcd.print("PIN: ____");
}

void checkPin() {
  if (enteredPin == correctPin) {
    disarmSystem();
  } else {
    wrongPinAlarm();
    startPinMode();
  }

  enteredPin = "";
}

void armSystem() {
  alarmOn = true;
  pinModeActive = false;
  movementScreenActive = false;
  enteredPin = "";
  showingNumbers = false;

  alarmStartTime = millis();
  lastMotionTime = 0;

  digitalWrite(ESP_ALARM_SIGNAL, HIGH);
  digitalWrite(ESP_MOVE_SIGNAL, LOW);

  doorLock.write(0);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ARMING SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("PLEASE WAIT");

  beepArmSound();

  delay(1000);
  showArmedScreen();
}

void disarmSystem() {
  alarmOn = false;
  pinModeActive = false;
  movementScreenActive = false;
  enteredPin = "";
  showingNumbers = false;

  digitalWrite(ESP_ALARM_SIGNAL, LOW);
  digitalWrite(ESP_MOVE_SIGNAL, LOW);

  digitalWrite(BUZZER, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  doorLock.write(90);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCESS GRANTED");
  lcd.setCursor(0, 1);
  lcd.print("ALARM DISARMED");

  beepDisarmSound();

  delay(1500);
  showDisarmedScreen();
}

void wrongPinAlarm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WRONG PIN");
  lcd.setCursor(0, 1);
  lcd.print("ACCESS DENIED");

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  for (int i = 0; i < 4; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(180);
    digitalWrite(BUZZER, LOW);
    delay(180);
  }
}

long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  long distance = duration * 0.034 / 2;

  if (distance < 2 || distance > 400) {
    return 999;
  }

  return distance;
}

void checkMovement() {
  if (!alarmOn) return;
  if (pinModeActive) return;
  if (millis() - alarmStartTime < warmupTime) return;
  if (millis() - lastSensorCheck < sensorCheckInterval) return;

  lastSensorCheck = millis();

  currentDistance = readDistanceCM();

  Serial.print("Distance: ");
  Serial.print(currentDistance);
  Serial.println(" cm");

  if (currentDistance == 999) {
    return;
  }

  if (currentDistance <= detectionDistance) {
    if (millis() - lastMotionTime >= motionCooldown || lastMotionTime == 0) {
      triggerMovement();
    }
  }
}

void triggerMovement() {
  lastMotionTime = millis();
  movementScreenStart = millis();
  espMoveSignalStart = millis();
  movementScreenActive = true;

  digitalWrite(ESP_MOVE_SIGNAL, HIGH);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MOVEMENT");
  lcd.setCursor(0, 1);
  lcd.print("DETECTED ");
  lcd.print(currentDistance);
  lcd.print("cm");

  digitalWrite(BUZZER, HIGH);
  delay(250);
  digitalWrite(BUZZER, LOW);
}

void updateArmedScreen() {
  if (!alarmOn) return;
  if (pinModeActive) return;
  if (movementScreenActive) return;
  if (millis() - lastLcdUpdate < lcdUpdateInterval) return;

  lastLcdUpdate = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM ARMED");

  lcd.setCursor(0, 1);
  lcd.print("NO MOVEMENT");
}

void updateMovementScreen() {
  if (!movementScreenActive) return;

  if (millis() - movementScreenStart >= movementScreenTime) {
    movementScreenActive = false;

    if (alarmOn && !pinModeActive) {
      showArmedScreen();
    }
  }
}

void updateEspMoveSignal() {
  if (digitalRead(ESP_MOVE_SIGNAL) == HIGH) {
    if (millis() - espMoveSignalStart >= espMoveSignalTime) {
      digitalWrite(ESP_MOVE_SIGNAL, LOW);
    }
  }
}

void showStartingScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART SECURITY");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM STARTING");
}

void showArmedScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM ARMED");
  lcd.setCursor(0, 1);
  lcd.print("NO MOVEMENT");
}

void showDisarmedScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART SECURITY");
  lcd.setCursor(0, 1);
  lcd.print("ALARM DISARMED");
}

void showPinNumbers() {
  lcd.setCursor(0, 1);
  lcd.print("PIN: ");
  lcd.print(enteredPin);

  for (int i = enteredPin.length(); i < 4; i++) {
    lcd.print("_");
  }

  lcd.print("       ");
}

void showPinStars() {
  lcd.setCursor(0, 1);
  lcd.print("PIN: ");

  for (int i = 0; i < enteredPin.length(); i++) {
    lcd.print("*");
  }

  for (int i = enteredPin.length(); i < 4; i++) {
    lcd.print("_");
  }

  lcd.print("       ");
}

void hideNumbersAfterDelay() {
  if (pinModeActive && showingNumbers) {
    if (millis() - lastDigitTime >= showNumbersTime) {
      showingNumbers = false;
      showPinStars();
    }
  }
}

void beepArmSound() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(150);
    digitalWrite(BUZZER, LOW);
    delay(150);
  }
}

void beepDisarmSound() {
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);
}

void beepShort() {
  digitalWrite(BUZZER, HIGH);
  delay(120);
  digitalWrite(BUZZER, LOW);
}