#include <Wire.h>
#include <SPI.h>
#include <ArduinoBLE.h>
#include <WiFiNINA.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Servo.h>
#include <FluxGarage_RoboEyes.h>

#ifdef W
#undef W
#endif
#ifdef N
#undef N
#endif
#ifdef E
#undef E
#endif
#ifdef S
#undef S
#endif

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

#define PIN_TOUCH 2
#define PIN_BUZZER 5
#define PIN_LDR A0

#define SERVO_1_PIN 9
#define SERVO_2_PIN 10
#define SERVO_3_PIN 11
#define SERVO_4_PIN 12

#define ENABLE_SERVOS true

#define MAX_REMINDERS 6

const int SCROLL_TEXT_SIZE = 2;
const int SCROLL_SPEED_MS = 25;
const int SCROLL_TEXT_Y = 24;

const unsigned long EYES_TIME = 180000;
const unsigned long CLOCK_TIME = 60000;
const unsigned long CHECK_DURATION = 2200;
const unsigned long PENDING_LIST_DURATION = 7000;

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;
RoboEyes<Adafruit_SH1106G> roboEyes(display);

Servo servo1, servo2, servo3, servo4;

BLEService ottoService("19B10000-E8F2-537E-4F6C-D104768A1214");

BLEStringCharacteristic commandChar(
  "19B10001-E8F2-537E-4F6C-D104768A1214",
  BLEWrite,
  180
);

BLEStringCharacteristic responseChar(
  "19B10002-E8F2-537E-4F6C-D104768A1214",
  BLERead | BLENotify,
  180
);

enum RobotState {
  STANDBY,
  SHOW_CHECK,
  SHOW_SCROLL_TEXT,
  SHOW_PENDING_LIST,
  REMINDER_ALERT
};

enum LightMode {
  LIGHT_DARK,
  LIGHT_MEDIUM,
  LIGHT_HIGH
};

struct Reminder {
  String id;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  String message;
  bool active;
  bool triggeredToday;
};

RobotState currentState = STANDBY;
Reminder reminders[MAX_REMINDERS];

String scrollMessage = "";
int scrollX = SCREEN_WIDTH;
unsigned long lastScrollUpdate = 0;
bool scrollAfterReminder = false;

unsigned long checkStartTime = 0;
unsigned long pendingListStart = 0;
unsigned long standbyTimer = 0;
unsigned long lastClockDraw = 0;

bool clockMode = false;

bool touching = false;
unsigned long touchStartTime = 0;
unsigned long lastPetAction = 0;
int currentMoodIndex = 0;

int ldrValue = 0;
bool nightMode = false;

// Bandera general para pausar WiFi en momentos delicados.
bool robotBusy = false;

// Prototipos globales
void enterStandby();
void updateStateMachine();

LightMode getLightMode();

void setupBLE();
void updateBLE();
void handleCommand(String cmd);

void setupDisplay();
void drawLargeCheck();
void startCheckAnimation();
void drawNotificationBell();
void startScrollText(String msg, bool afterReminder = false);
void updateScrollText();
void drawDigitalClock();
void showPendingList();
void updatePendingList();
void updateStandbyMode();
void printBoldText(String texto, int x, int y, int size);

void setupServos();
void reminderMovement();
bool handleServoCommand(String type);

void updateLDR();
void applyLdrBehavior();
void updateTouch();
void updateTouchClockMode();
void changeMoodByPet();

void setupBuzzer();
void beep(int freq, int duration, int pauseMs = 30);
void soundSuccess();
void soundBluetoothConnected();
void soundDefaultRobot();
void soundHappyRobot();
void soundTiredRobot();
void soundAngryRobot();
void soundReminder();

void saveReminder(String id, String date, String time, String msg, bool editMode);
void deleteReminder(String id);
void clearReminders();
void checkReminders();
void triggerReminder(Reminder r);

String normalizarTexto(String texto);
String getPart(String data, char separator, int index);
bool parseDate(String date, int &year, int &month, int &day);
bool parseTime(String time, int &hour, int &minute);
void logEvent(String action);

// WiFi
void setupWiFiEvents();
void updateWiFiEvents();
void queueEventLog(String actionType, String message);
void queueEventLogFull(String actionType, String message, String command, String robotResponse);
void testWiFiEvent();

void enterStandby() {
  currentState = STANDBY;
  clockMode = false;
  standbyTimer = millis();
  scrollAfterReminder = false;

  display.clearDisplay();
  display.display();
}

void updateStateMachine() {
  if (currentState == STANDBY) {
    updateStandbyMode();
    return;
  }

  if (currentState == SHOW_CHECK) {
    if (millis() - checkStartTime >= CHECK_DURATION) {
      enterStandby();
    }
    return;
  }

  if (currentState == SHOW_SCROLL_TEXT) {
    updateScrollText();
    return;
  }

  if (currentState == SHOW_PENDING_LIST) {
    updatePendingList();
    return;
  }

  if (currentState == REMINDER_ALERT) {
    return;
  }
}

void setup() {
  Serial.begin(115200);
  delay(800);

  randomSeed(analogRead(PIN_LDR));

  Wire.begin();
  Wire.setClock(100000);

  setupBuzzer();
  setupDisplay();

  if (!rtc.begin()) {
    Serial.println("ERROR RTC");
    while (1);
  }

  // USAR SOLO UNA VEZ PARA AJUSTAR HORA DEL RTC:
   //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  setupBLE();

  // WiFi va después de BLE para no retrasar la aparición del robot por Bluetooth.
  setupWiFiEvents();

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 45);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(ON);

  pinMode(PIN_TOUCH, INPUT);

  setupServos();

  enterStandby();

  logEvent("OttoBot iniciado");

  // Primer log para validar que el dashboard recibe eventos.
  queueEventLog("SYSTEM_START", "OttoBot iniciado");


  soundSuccess();
}

void loop() {
  updateBLE();
  updateSerialCommands();

  updateLDR();
  updateTouch();
  checkReminders();
  updateStateMachine();

  updateWiFiEvents();

}