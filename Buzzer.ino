// Buzzer.ino - OttoBot Agenda
// Sonidos roboticos variados


// Este archivo maneja SOLO sonidos.
// Los movimientos sincronizados se hacen en Servos.ino.
// Servos.ino usa:
//   buzzerStartTone()
//   buzzerStopTone()
//   buzzerHoldTone()


// -----------------------------------------------------
// Notas musicales base
// -----------------------------------------------------
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494

#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988

#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_G6  1568
#define NOTE_A6  1760
#define NOTE_B6  1976

// Delay con mantenimiento BLE

void buzzerSmartDelay(unsigned long ms) {
  unsigned long start = millis();

  while (millis() - start < ms) {
    BLE.poll();
    delay(4);
  }
}


// Setup del buzzer
void setupBuzzer() {
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.println("BUZZER: listo");
}



// Funciones base del buzzer
void buzzerStartTone(int freq) {
  tone(PIN_BUZZER, freq);
}


void buzzerStopTone() {
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BUZZER, LOW);
}


void buzzerHoldTone(int freq, int durationMs) {
  tone(PIN_BUZZER, freq);
  buzzerSmartDelay(durationMs);
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BUZZER, LOW);
  buzzerSmartDelay(18);
}


void beep(int freq, int duration, int pauseMs) {
  buzzerHoldTone(freq, duration);
  buzzerSmartDelay(pauseMs);
}


void playNote(int freq, int duration) {
  buzzerHoldTone(freq, duration);
}



// Sonidos cortos normales del robot

void soundSuccess() {
  buzzerHoldTone(1100, 80);
  buzzerHoldTone(1500, 90);
  buzzerHoldTone(1900, 130);
}


void soundBluetoothConnected() {
  buzzerHoldTone(1000, 70);
  buzzerHoldTone(1400, 90);
}


void soundDefaultRobot() {
  buzzerHoldTone(900, 80);
  buzzerHoldTone(1200, 80);
}


void soundHappyRobot() {
  buzzerHoldTone(1200, 80);
  buzzerHoldTone(1600, 80);
  buzzerHoldTone(2000, 120);
}


void soundTiredRobot() {
  buzzerHoldTone(600, 120);
  buzzerHoldTone(450, 160);
}


void soundAngryRobot() {
  buzzerHoldTone(300, 90);
  buzzerHoldTone(260, 90);
  buzzerHoldTone(300, 120);
}


void soundReminderSoft() {
  buzzerHoldTone(900, 120);
  buzzerHoldTone(1200, 120);
  buzzerHoldTone(1500, 160);
}

// Sonidos solos variados para pruebas o TEST_ALL

void soundScannerSolo() {
  Serial.println("BUZZER: scanner solo");

  buzzerHoldTone(500, 90);
  buzzerHoldTone(750, 90);
  buzzerHoldTone(1100, 100);
  buzzerHoldTone(1500, 120);
  buzzerHoldTone(1900, 140);

  buzzerSmartDelay(50);

  buzzerHoldTone(1700, 100);
  buzzerHoldTone(1300, 100);
  buzzerHoldTone(900, 120);
  buzzerHoldTone(600, 160);
}


void soundCuteSolo() {
  Serial.println("BUZZER: cute solo");

  buzzerHoldTone(1400, 80);
  buzzerHoldTone(1800, 80);
  buzzerHoldTone(2300, 110);

  buzzerSmartDelay(50);

  buzzerHoldTone(1700, 90);
  buzzerHoldTone(2100, 90);
  buzzerHoldTone(2600, 130);

  buzzerSmartDelay(60);

  buzzerHoldTone(2000, 100);
  buzzerHoldTone(1500, 150);
}


void soundIndustrialSolo() {
  Serial.println("BUZZER: industrial solo");

  buzzerHoldTone(180, 130);
  buzzerHoldTone(420, 80);
  buzzerHoldTone(220, 140);
  buzzerHoldTone(520, 80);
  buzzerHoldTone(260, 150);
  buzzerHoldTone(700, 90);
  buzzerHoldTone(300, 190);
}


void soundVictorySolo() {
  Serial.println("BUZZER: victory solo");

  buzzerHoldTone(800, 90);
  buzzerHoldTone(1100, 90);
  buzzerHoldTone(1500, 100);
  buzzerHoldTone(2000, 150);

  buzzerSmartDelay(60);

  buzzerHoldTone(1600, 100);
  buzzerHoldTone(2100, 120);
  buzzerHoldTone(2600, 180);

  buzzerSmartDelay(70);

  buzzerHoldTone(1900, 100);
  buzzerHoldTone(1400, 180);
}


void soundSuspenseSolo() {
  Serial.println("BUZZER: suspense solo");

  buzzerHoldTone(600, 180);
  buzzerHoldTone(680, 180);
  buzzerHoldTone(620, 180);
  buzzerHoldTone(760, 220);

  buzzerSmartDelay(80);

  buzzerHoldTone(900, 180);
  buzzerHoldTone(1100, 220);
  buzzerHoldTone(1400, 280);
}


// Sonido de recordatorio general

void soundReminder() {
  int option = random(0, 5);

  if (option == 0) {
    soundScannerSolo();
  } else if (option == 1) {
    soundCuteSolo();
  } else if (option == 2) {
    soundIndustrialSolo();
  } else if (option == 3) {
    soundVictorySolo();
  } else {
    soundSuspenseSolo();
  }

  buzzerStopTone();
}