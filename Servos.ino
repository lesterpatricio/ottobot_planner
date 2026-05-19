
// Servos.ino OttoBot Agenda
// Movimientos roboticos

// Comandos:
//   SHOW_SCAN
//   SHOW_CUTE
//   SHOW_WIN
//   SHOW_KICK
//   SHOW_BOW
//   SHOW_STOMP
//   SHOW_ONE_FOOT
//   SHOW_RANDOM
//   SHOW_FULL

// Funciones usadas por BLEModule.ino:
//   testWalkServos()
//   testWalkStronger()
//   testServos()
//   handleServoCommand(String type)
//
// Funciones usadas por Reminders.ino:
//   reminderAgendaMovementStrong()
//   reminderAgendaMovementMedium()

// AUTORIZACION DE MOVIMIENTO PARA RECORDATORIOS
bool reminderMotionAuthorized = false;

// CALIBRACION FINA DE SERVOS
// Servo fisico 1 = pie derecho
// Servo fisico 2 = pie izquierdo
// Servo fisico 3 = pierna/cadera derecha
// Servo fisico 4 = pierna/cadera izquierda

// Mapeo logico usado en este archivo:

// LL = pierna izquierda  -> SERVO_4_PIN -> offset 8
// RL = pierna derecha    -> SERVO_3_PIN -> offset 0
// LF = pie izquierdo     -> SERVO_2_PIN -> offset 0
// RF = pie derecho       -> SERVO_1_PIN -> offset 15

const int OFFSET_LL = 8;
const int OFFSET_RL = 0;
const int OFFSET_LF = 0;
const int OFFSET_RF = 15;

// LIMITES SEGUROS

const int SERVO_MIN_SAFE = 55;
const int SERVO_MAX_SAFE = 125;

// POSICION ACTUAL INTERNA

int posLL = 90;
int posRL = 90;
int posLF = 90;
int posRF = 90;

// FRECUENCIAS PARA EFECTOS
const int T_C4 = 262;
const int T_D4 = 294;
const int T_E4 = 330;
const int T_F4 = 349;
const int T_G4 = 392;
const int T_A4 = 440;
const int T_B4 = 494;

const int T_C5 = 523;
const int T_D5 = 587;
const int T_E5 = 659;
const int T_F5 = 698;
const int T_G5 = 784;
const int T_A5 = 880;
const int T_B5 = 988;

const int T_C6 = 1047;
const int T_D6 = 1175;
const int T_E6 = 1319;
const int T_F6 = 1397;
const int T_G6 = 1568;
const int T_A6 = 1760;
const int T_B6 = 1976;

// UTILIDADES INTERNAS

int safeAngle(int angle) {
  if (angle < SERVO_MIN_SAFE) return SERVO_MIN_SAFE;
  if (angle > SERVO_MAX_SAFE) return SERVO_MAX_SAFE;
  return angle;
}


void servoSmartDelay(unsigned long ms) {
  unsigned long start = millis();

  while (millis() - start < ms) {
    BLE.poll();
    delay(4);
  }
}


void attachServosIfNeeded() {
  if (!servo1.attached()) servo1.attach(SERVO_4_PIN);
  if (!servo2.attached()) servo2.attach(SERVO_3_PIN);
  if (!servo3.attached()) servo3.attach(SERVO_2_PIN);
  if (!servo4.attached()) servo4.attach(SERVO_1_PIN);
}


void writeServosRaw(int ll, int rl, int lf, int rf) {
  posLL = safeAngle(ll);
  posRL = safeAngle(rl);
  posLF = safeAngle(lf);
  posRF = safeAngle(rf);

  servo1.write(safeAngle(posLL + OFFSET_LL));
  servo2.write(safeAngle(posRL + OFFSET_RL));
  servo3.write(safeAngle(posLF + OFFSET_LF));
  servo4.write(safeAngle(posRF + OFFSET_RF));
}


void moveServosSmooth(int targetLL, int targetRL, int targetLF, int targetRF, int durationMs) {
  attachServosIfNeeded();

  targetLL = safeAngle(targetLL);
  targetRL = safeAngle(targetRL);
  targetLF = safeAngle(targetLF);
  targetRF = safeAngle(targetRF);

  int startLL = posLL;
  int startRL = posRL;
  int startLF = posLF;
  int startRF = posRF;

  int steps = durationMs / 20;
  if (steps < 1) steps = 1;

  for (int i = 1; i <= steps; i++) {
    int newLL = startLL + ((targetLL - startLL) * i) / steps;
    int newRL = startRL + ((targetRL - startRL) * i) / steps;
    int newLF = startLF + ((targetLF - startLF) * i) / steps;
    int newRF = startRF + ((targetRF - startRF) * i) / steps;

    writeServosRaw(newLL, newRL, newLF, newRF);
    servoSmartDelay(20);
  }
}


void silentMove(int durationMs, int ll, int rl, int lf, int rf) {
  moveServosSmooth(ll, rl, lf, rf, durationMs);
  servoSmartDelay(20);
}


void toneMove(int freq, int durationMs, int ll, int rl, int lf, int rf) {
  buzzerStartTone(freq);
  moveServosSmooth(ll, rl, lf, rf, durationMs);
  buzzerStopTone();
  servoSmartDelay(20);
}


void toneMoveSweep(int startFreq, int endFreq, int durationMs, int ll, int rl, int lf, int rf) {
  attachServosIfNeeded();

  ll = safeAngle(ll);
  rl = safeAngle(rl);
  lf = safeAngle(lf);
  rf = safeAngle(rf);

  int startLL = posLL;
  int startRL = posRL;
  int startLF = posLF;
  int startRF = posRF;

  int steps = durationMs / 20;
  if (steps < 1) steps = 1;

  for (int i = 1; i <= steps; i++) {
    int freq = startFreq + ((endFreq - startFreq) * i) / steps;

    int newLL = startLL + ((ll - startLL) * i) / steps;
    int newRL = startRL + ((rl - startRL) * i) / steps;
    int newLF = startLF + ((lf - startLF) * i) / steps;
    int newRF = startRF + ((rf - startRF) * i) / steps;

    buzzerStartTone(freq);
    writeServosRaw(newLL, newRL, newLF, newRF);
    servoSmartDelay(20);
  }

  buzzerStopTone();
  servoSmartDelay(20);
}


void toneMovePulse(int f1, int f2, int durationMs, int ll, int rl, int lf, int rf) {
  attachServosIfNeeded();

  ll = safeAngle(ll);
  rl = safeAngle(rl);
  lf = safeAngle(lf);
  rf = safeAngle(rf);

  int startLL = posLL;
  int startRL = posRL;
  int startLF = posLF;
  int startRF = posRF;

  int steps = durationMs / 20;
  if (steps < 1) steps = 1;

  for (int i = 1; i <= steps; i++) {
    if (i % 2 == 0) {
      buzzerStartTone(f1);
    } else {
      buzzerStartTone(f2);
    }

    int newLL = startLL + ((ll - startLL) * i) / steps;
    int newRL = startRL + ((rl - startRL) * i) / steps;
    int newLF = startLF + ((lf - startLF) * i) / steps;
    int newRF = startRF + ((rf - startRF) * i) / steps;

    writeServosRaw(newLL, newRL, newLF, newRF);
    servoSmartDelay(20);
  }

  buzzerStopTone();
  servoSmartDelay(20);
}


void toneMoveArcade(int f1, int f2, int f3, int durationMs, int ll, int rl, int lf, int rf) {
  attachServosIfNeeded();

  ll = safeAngle(ll);
  rl = safeAngle(rl);
  lf = safeAngle(lf);
  rf = safeAngle(rf);

  int startLL = posLL;
  int startRL = posRL;
  int startLF = posLF;
  int startRF = posRF;

  int steps = durationMs / 20;
  if (steps < 3) steps = 3;

  for (int i = 1; i <= steps; i++) {
    if (i < steps / 3) {
      buzzerStartTone(f1);
    } else if (i < (steps * 2) / 3) {
      buzzerStartTone(f2);
    } else {
      buzzerStartTone(f3);
    }

    int newLL = startLL + ((ll - startLL) * i) / steps;
    int newRL = startRL + ((rl - startRL) * i) / steps;
    int newLF = startLF + ((lf - startLF) * i) / steps;
    int newRF = startRF + ((rf - startRF) * i) / steps;

    writeServosRaw(newLL, newRL, newLF, newRF);
    servoSmartDelay(20);
  }

  buzzerStopTone();
  servoSmartDelay(20);
}


void toneMoveStutter(int freq, int durationMs, int ll, int rl, int lf, int rf) {
  attachServosIfNeeded();

  int steps = durationMs / 40;
  if (steps < 1) steps = 1;

  for (int i = 0; i < steps; i++) {
    buzzerStartTone(freq);
    moveServosSmooth(ll, rl, lf, rf, 20);
    buzzerStopTone();
    servoSmartDelay(20);
  }
}


void toneMoveDrum(int durationMs, int ll, int rl, int lf, int rf) {
  buzzerStartTone(260);
  moveServosSmooth(ll, rl, lf, rf, durationMs);
  buzzerStopTone();
  servoSmartDelay(30);
}


void toneMoveCute(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveArcade(1600, 2200, 2700, durationMs, ll, rl, lf, rf);
}


void toneMoveVictory(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveArcade(900, 1500, 2400, durationMs, ll, rl, lf, rf);
}


void toneMoveSuspense(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveSweep(500, 950, durationMs, ll, rl, lf, rf);
}


void toneMoveLaser(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveSweep(500, 1800, durationMs, ll, rl, lf, rf);
}


void toneMoveLaserDown(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveSweep(1800, 500, durationMs, ll, rl, lf, rf);
}

// CENTRADO
void centerServosSoft() {
  silentMove(600, 90, 90, 90, 90);
}

void centerServosFast() {
  silentMove(250, 90, 90, 90, 90);
}

// CONFIGURACION SERVOS

void setupServos() {
  if (!ENABLE_SERVOS) {
    Serial.println("SERVOS: desactivados por configuracion");
    return;
  }

  servo1.attach(SERVO_4_PIN);
  servo2.attach(SERVO_3_PIN);
  servo3.attach(SERVO_2_PIN);
  servo4.attach(SERVO_1_PIN);

  centerServosSoft();

  Serial.println("SERVOS: listos");
}

// DETACH SERVOS

void detachServos() {
  servo1.detach();
  servo2.detach();
  servo3.detach();
  servo4.detach();

  Serial.println("SERVOS: detach");
}

// SHOW_SCAN
void showRobotScanner() {
  Serial.println("SHOW: ROBOT_SCANNER");

  attachServosIfNeeded();
  centerServosSoft();

  toneMoveLaser(320, 78, 102, 76, 104);
  servoSmartDelay(180);

  toneMoveSweep(600, 1900, 650, 62, 118, 82, 98);
  servoSmartDelay(250);

  toneMoveSweep(1900, 600, 650, 118, 62, 98, 82);
  servoSmartDelay(250);

  toneMovePulse(700, 1500, 280, 82, 98, 72, 108);
  toneMovePulse(1500, 700, 280, 98, 82, 108, 72);

  for (int i = 0; i < 2; i++) {
    toneMoveStutter(1500, 180, 76, 104, 86, 94);
    toneMoveStutter(1000, 180, 104, 76, 94, 86);
  }

  toneMoveLaserDown(360, 90, 90, 90, 90);
  centerServosSoft();

  Serial.println("SHOW_SCAN terminado");
}

// SHOW_CUTE
void showCuteRobotAlarm() {
  Serial.println("SHOW: CUTE_ROBOT_ALARM");

  attachServosIfNeeded();
  centerServosSoft();

  for (int i = 0; i < 3; i++) {
    toneMoveCute(240, 84, 96, 78, 102);
    toneMoveCute(240, 96, 84, 102, 78);
  }

  toneMoveArcade(1600, 2100, 2600, 380, 76, 104, 86, 94);
  servoSmartDelay(200);

  toneMoveArcade(2600, 2100, 1600, 380, 104, 76, 94, 86);
  servoSmartDelay(200);

  for (int i = 0; i < 2; i++) {
    toneMoveCute(180, 86, 94, 84, 96);
    toneMoveCute(180, 94, 86, 96, 84);
  }

  toneMoveArcade(1800, 2300, 2800, 360, 90, 90, 76, 104);
  toneMoveArcade(2800, 2300, 1800, 360, 90, 90, 104, 76);

  centerServosSoft();

  Serial.println("SHOW_CUTE terminado");
}

// SHOW_WIN
void showVictoryRobotDance() {
  Serial.println("SHOW: VICTORY_ROBOT_DANCE");

  attachServosIfNeeded();
  centerServosSoft();

  for (int i = 0; i < 3; i++) {
    toneMoveVictory(240, 78, 102, 80, 100);
    toneMoveVictory(240, 102, 78, 100, 80);
  }

  toneMoveArcade(900, 1400, 2000, 480, 72, 108, 84, 96);
  toneMoveArcade(2000, 1400, 900, 480, 108, 72, 96, 84);

  toneMovePulse(1000, 1800, 500, 80, 100, 68, 112);
  servoSmartDelay(350);

  toneMovePulse(1800, 1000, 500, 100, 80, 112, 68);
  servoSmartDelay(350);

  toneMoveArcade(1200, 1600, 2100, 420, 84, 96, 80, 100);
  toneMoveArcade(2100, 1600, 1200, 420, 96, 84, 100, 80);

  centerServosSoft();

  Serial.println("SHOW_WIN terminado");
}

// SHOW_KICK
void showRobotKicks() {
  Serial.println("SHOW: ROBOT_KICKS");

  attachServosIfNeeded();
  centerServosSoft();

  Serial.println("KICK: derecha suave");

  toneMoveArcade(500, 850, 1200, 520, 104, 76, 100, 80);
  servoSmartDelay(180);

  toneMoveSweep(700, 1300, 520, 110, 70, 106, 70);
  servoSmartDelay(180);

  toneMovePulse(1100, 1700, 380, 112, 68, 114, 62);
  servoSmartDelay(250);

  toneMoveSweep(1300, 800, 500, 106, 74, 102, 78);
  servoSmartDelay(200);

  silentMove(500, 96, 84, 96, 84);
  servoSmartDelay(250);

  Serial.println("KICK: izquierda suave");

  toneMoveArcade(500, 850, 1200, 520, 76, 104, 80, 100);
  servoSmartDelay(180);

  toneMoveSweep(700, 1300, 520, 70, 110, 70, 106);
  servoSmartDelay(180);

  toneMovePulse(1100, 1700, 380, 68, 112, 62, 114);
  servoSmartDelay(250);

  toneMoveSweep(1300, 800, 500, 74, 106, 78, 102);
  servoSmartDelay(200);

  silentMove(500, 84, 96, 84, 96);
  silentMove(500, 90, 90, 90, 90);

  centerServosSoft();

  Serial.println("SHOW_KICK terminado");
}

// SHOW_BOW
void showRobotBow() {
  Serial.println("SHOW: ROBOT_BOW_GREETING");

  attachServosIfNeeded();
  centerServosSoft();

  toneMoveCute(360, 82, 98, 78, 102);
  servoSmartDelay(200);

  toneMoveCute(360, 98, 82, 102, 78);
  servoSmartDelay(200);

  toneMoveArcade(700, 1000, 1400, 450, 90, 90, 74, 106);
  servoSmartDelay(250);

  toneMoveSweep(800, 1350, 650, 74, 106, 80, 100);
  servoSmartDelay(550);

  toneMoveCute(260, 78, 102, 84, 96);
  toneMoveCute(260, 74, 106, 80, 100);
  servoSmartDelay(350);

  toneMoveSweep(1350, 900, 600, 96, 84, 98, 82);
  servoSmartDelay(250);

  toneMoveCute(250, 84, 96, 78, 102);
  toneMoveCute(250, 96, 84, 102, 78);

  centerServosSoft();

  Serial.println("SHOW_BOW terminado");
}

// SHOW_STOMP - Pisoton robotico claro
void showRobotStomp() {
  Serial.println("SHOW: ROBOT_STOMP_REDESIGN");

  attachServosIfNeeded();
  centerServosSoft();
  servoSmartDelay(500);

  // Pisoton derecho
  Serial.println("STOMP: derecho");

  // Inclina peso a la izquierda para liberar lado derecho
  toneMoveDrum(350, 106, 74, 96, 84);
  servoSmartDelay(180);

  // Levanta/descarga pie derecho
  toneMovePulse(500, 750, 320, 110, 70, 100, 70);
  servoSmartDelay(180);

  // Pisoton derecho: regresa al centro con golpe
  buzzerStartTone(220);
  silentMove(180, 90, 90, 90, 90);
  buzzerStopTone();
  servoSmartDelay(350);

  // Pisoton izquierdo
  Serial.println("STOMP: izquierdo");

  // Inclina peso a la derecha para liberar lado izquierdo
  toneMoveDrum(350, 74, 106, 84, 96);
  servoSmartDelay(180);

  // Levanta/descarga pie izquierdo
  toneMovePulse(500, 750, 320, 70, 110, 70, 100);
  servoSmartDelay(180);

  // Pisoton izquierdo: regresa al centro con golpe
  buzzerStartTone(220);
  silentMove(180, 90, 90, 90, 90);
  buzzerStopTone();
  servoSmartDelay(350);

  // Doble pisoton final, mas vistoso pero controlado
  Serial.println("STOMP: doble final");

  // Abre postura
  toneMovePulse(450, 700, 350, 78, 102, 78, 102);
  servoSmartDelay(200);

  // Golpe al centro
  buzzerStartTone(180);
  silentMove(180, 90, 90, 90, 90);
  buzzerStopTone();
  servoSmartDelay(250);

  // Abre al otro lado
  toneMovePulse(500, 750, 350, 102, 78, 102, 78);
  servoSmartDelay(200);

  // Golpe final al centro
  buzzerStartTone(180);
  silentMove(180, 90, 90, 90, 90);
  buzzerStopTone();
  servoSmartDelay(250);

  centerServosSoft();

  Serial.println("SHOW_STOMP terminado");
}


// SHOW_ONE_FOOT
void showOneFootBalance() {
  Serial.println("SHOW: ONE_FOOT_ELEVATION_DANCE_MORE");

  attachServosIfNeeded();
  centerServosSoft();
  servoSmartDelay(250);

  // 1. Junta pies
  Serial.println("ONE_FOOT: juntar pies");
  silentMove(300, 90, 90, 110, 70);
  servoSmartDelay(100);

  // 2. Precarga corta
  toneMoveSweep(500, 700, 240, 88, 92, 116, 64);
  servoSmartDelay(80);

  // 3. Subida rapida y visible
  Serial.println("ONE_FOOT: subir");

  toneMoveSweep(700, 1000, 260, 84, 96, 122, 58);
  servoSmartDelay(80);

  toneMoveSweep(1000, 1150, 220, 82, 98, 125, 55);

  // 4. Mantener pose alta
  Serial.println("ONE_FOOT: sostener pose alta");

  buzzerStartTone(380);
  silentMove(180, 82, 98, 125, 55);
  buzzerStopTone();

  servoSmartDelay(350);

  // 5. Baile arriba mas vistoso
  Serial.println("ONE_FOOT: baile arriba vistoso");

  // Balance lateral 1
  buzzerStartTone(420);
  silentMove(160, 78, 102, 125, 55);
  buzzerStopTone();
  servoSmartDelay(80);

  buzzerStartTone(520);
  silentMove(160, 86, 94, 122, 58);
  buzzerStopTone();
  servoSmartDelay(80);

  // Balance lateral 2
  buzzerStartTone(460);
  silentMove(160, 80, 100, 125, 55);
  buzzerStopTone();
  servoSmartDelay(80);

  buzzerStartTone(560);
  silentMove(160, 84, 96, 123, 57);
  buzzerStopTone();
  servoSmartDelay(80);

  // Rebote arriba
  buzzerStartTone(620);
  silentMove(150, 82, 98, 125, 55);
  buzzerStopTone();
  servoSmartDelay(70);

  buzzerStartTone(520);
  silentMove(150, 86, 94, 121, 59);
  buzzerStopTone();
  servoSmartDelay(70);

  buzzerStartTone(680);
  silentMove(150, 82, 98, 125, 55);
  buzzerStopTone();
  servoSmartDelay(120);

  // Mini shake final arriba
  for (int i = 0; i < 2; i++) {
    buzzerStartTone(500);
    silentMove(120, 80, 100, 124, 56);
    buzzerStopTone();
    servoSmartDelay(50);

    buzzerStartTone(650);
    silentMove(120, 84, 96, 125, 55);
    buzzerStopTone();
    servoSmartDelay(50);
  }

  // Pose final antes de bajar
  buzzerStartTone(720);
  silentMove(180, 82, 98, 125, 55);
  buzzerStopTone();
  servoSmartDelay(250);

  // 6. Bajar ambos pies al mismo tiempo, fluido
  Serial.println("ONE_FOOT: bajar ambos pies");

  toneMoveSweep(750, 520, 420, 86, 94, 116, 64);
  servoSmartDelay(60);

  toneMoveSweep(520, 420, 360, 88, 92, 108, 72);
  servoSmartDelay(60);

  // Recuperacion final suave
  silentMove(320, 90, 90, 98, 82);
  silentMove(260, 90, 90, 90, 90);

  centerServosSoft();

  Serial.println("SHOW_ONE_FOOT terminado");
}


// SHOW_FULL
void showFullExhibition() {
  Serial.println("SHOW: FULL_EXHIBITION");

  attachServosIfNeeded();
  centerServosSoft();

  showRobotScanner();
  servoSmartDelay(250);

  showCuteRobotAlarm();
  servoSmartDelay(250);

  showRobotStomp();
  servoSmartDelay(250);

  showRobotKicks();
  servoSmartDelay(250);

  showOneFootBalance();
  servoSmartDelay(250);

  showRobotBow();
  servoSmartDelay(250);

  showVictoryRobotDance();

  centerServosSoft();

  Serial.println("SHOW_FULL terminado");
}


// SHOW_RANDOM
void playRandomShowcaseMove() {
  int option = random(0, 7);

  if (option == 0) {
    showRobotScanner();
  }

  else if (option == 1) {
    showCuteRobotAlarm();
  }

  else if (option == 2) {
    showVictoryRobotDance();
  }

  else if (option == 3) {
    showRobotKicks();
  }

  else if (option == 4) {
    showRobotBow();
  }

  else if (option == 5) {
    showRobotStomp();
  }

  else {
    showOneFootBalance();
  }
}


// MOVIMIENTO PARA RECORDATORIO FUERTE
void reminderAgendaMovementStrong() {
  if (!reminderMotionAuthorized) {
    Serial.println("SERVOS: movimiento fuerte bloqueado");
    return;
  }

  attachServosIfNeeded();

  Serial.println("REMINDER MOVEMENT STRONG");

  showRobotScanner();
  servoSmartDelay(300);
  showVictoryRobotDance();

  centerServosSoft();
}


// MOVIMIENTO PARA RECORDATORIO MEDIO

void reminderAgendaMovementMedium() {
  if (!reminderMotionAuthorized) {
    Serial.println("SERVOS: movimiento medio bloqueado");
    return;
  }

  attachServosIfNeeded();

  Serial.println("REMINDER MOVEMENT MEDIUM");

  showCuteRobotAlarm();

  centerServosSoft();
}


// MOVIMIENTO LEGACY PARA RECORDATORIO

void reminderMovement() {
  reminderMotionAuthorized = true;
  reminderAgendaMovementStrong();
  reminderMotionAuthorized = false;
}


// PRUEBA GENERAL DE SERVOS
void testServos() {
  attachServosIfNeeded();

  Serial.println("TEST SERVOS");

  centerServosSoft();

  silentMove(500, 75, 105, 90, 90);
  silentMove(500, 105, 75, 90, 90);
  silentMove(500, 90, 90, 75, 105);
  silentMove(500, 90, 90, 105, 75);

  centerServosSoft();

  Serial.println("TEST SERVOS terminado");
}

// TEST WALK SERVOS
void testWalkServos() {
  attachServosIfNeeded();

  Serial.println("TEST WALK SERVOS");

  centerServosSoft();

  for (int i = 0; i < 3; i++) {
    silentMove(380, 78, 102, 78, 102);
    servoSmartDelay(120);

    silentMove(380, 102, 78, 102, 78);
    servoSmartDelay(120);
  }

  centerServosSoft();

  Serial.println("TEST WALK SERVOS terminado");
}


// TEST WALK STRONGER
void testWalkStronger() {
  attachServosIfNeeded();

  Serial.println("TEST WALK STRONGER");

  centerServosSoft();

  for (int i = 0; i < 3; i++) {
    silentMove(320, 70, 110, 72, 108);
    servoSmartDelay(120);

    silentMove(320, 110, 70, 108, 72);
    servoSmartDelay(120);
  }

  centerServosSoft();

  Serial.println("TEST WALK STRONGER terminado");
}

// MANEJO DE COMANDOS DE SERVOS
bool handleServoCommand(String type) {
  type.trim();
  type.toUpperCase();

  if (type == "SHOW_SCAN" || type == "SCAN") {
    showRobotScanner();
    return true;
  }

  if (type == "SHOW_CUTE" || type == "CUTE") {
    showCuteRobotAlarm();
    return true;
  }

  if (type == "SHOW_WIN" || type == "WIN") {
    showVictoryRobotDance();
    return true;
  }

  if (type == "SHOW_KICK" || type == "KICK") {
    showRobotKicks();
    return true;
  }

  if (type == "SHOW_BOW" || type == "BOW") {
    showRobotBow();
    return true;
  }

  if (type == "SHOW_STOMP" || type == "STOMP") {
    showRobotStomp();
    return true;
  }

  if (type == "SHOW_ONE_FOOT" || type == "ONE_FOOT") {
    showOneFootBalance();
    return true;
  }

  if (type == "SHOW_RANDOM" || type == "RANDOM") {
    playRandomShowcaseMove();
    return true;
  }

  if (type == "SHOW_FULL" || type == "FULL") {
    showFullExhibition();
    return true;
  }

  if (type == "TEST_SERVOS") {
    testServos();
    return true;
  }

  if (type == "TEST_WALK") {
    testWalkServos();
    return true;
  }

  if (type == "TEST_WALK_STRONGER") {
    testWalkStronger();
    return true;
  }

  return false;
}