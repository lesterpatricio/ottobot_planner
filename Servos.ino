// Servos.ino - OttoBot Agenda
// Movimientos roboticos variados + sonidos sincronizados

// Usa funciones de Buzzer.ino:
//   buzzerStartTone(freq)
//   buzzerStopTone()

// Compatible con Reminders.ino:
//   reminderAgendaMovementStrong()
//   reminderAgendaMovementMedium()

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



// Seguro de movimiento de recordatorio.
// Reminders.ino lo activa solo cuando hay una alarma real.
bool reminderMotionAuthorized = false;


// IMPORTANTE:
// SERVO_1_PIN = pin fisico donde esta conectado el servo 1
// SERVO_2_PIN = pin fisico donde esta conectado el servo 2
// SERVO_3_PIN = pin fisico donde esta conectado el servo 3
// SERVO_4_PIN = pin fisico donde esta conectado el servo 4

// Servo fisico 1 = pie derecho
// Servo fisico 2 = pie izquierdo
// Servo fisico 3 = pierna/cadera derecha
// Servo fisico 4 = pierna/cadera izquierda

// Entonces:
// LL = pierna izquierda  -> SERVO_4_PIN -> offset 8
// RL = pierna derecha    -> SERVO_3_PIN -> offset 0
// LF = pie izquierdo     -> SERVO_2_PIN -> offset 0
// RF = pie derecho       -> SERVO_1_PIN -> offset 15

//OFFSET para calibrar algun servo en mala posicion
const int OFFSET_LL = 8;   // Pierna izquierda real
const int OFFSET_RL = 0;   // Pierna derecha real
const int OFFSET_LF = 0;   // Pie izquierdo real
const int OFFSET_RF = 15;  // Pie derecho real


// Limites seguros.
// Para feria uso 55 a 125 para que se vea mas vistoso.
// Si el robot fuerza servos o se cae mucho, se usa 60 y 120.
const int SERVO_MIN_SAFE = 55;
const int SERVO_MAX_SAFE = 125;


// Posicion actual interna.
int posLL = 90;
int posRL = 90;
int posLF = 90;
int posRF = 90;

// Frecuencias base para efectos
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


// Utilidades internas
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

void writeServosRaw(int ll, int rl, int lf, int rf) {
  posLL = safeAngle(ll);
  posRL = safeAngle(rl);
  posLF = safeAngle(lf);
  posRF = safeAngle(rf);


  // servo1 = pierna izquierda
  // servo2 = pierna derecha
  // servo3 = pie izquierdo
  // servo4 = pie derecho

  servo1.write(safeAngle(posLL + OFFSET_LL));
  servo2.write(safeAngle(posRL + OFFSET_RL));
  servo3.write(safeAngle(posLF + OFFSET_LF));
  servo4.write(safeAngle(posRF + OFFSET_RF));
}


void moveServosSmooth(int targetLL, int targetRL, int targetLF, int targetRF, int durationMs) {
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
    int ll = startLL + ((targetLL - startLL) * i) / steps;
    int rl = startRL + ((targetRL - startRL) * i) / steps;
    int lf = startLF + ((targetLF - startLF) * i) / steps;
    int rf = startRF + ((targetRF - startRF) * i) / steps;

    writeServosRaw(ll, rl, lf, rf);
    servoSmartDelay(20);
  }
}

// Movimiento + tono simple
void toneMove(int freq, int durationMs, int ll, int rl, int lf, int rf) {
  buzzerStartTone(freq);
  moveServosSmooth(ll, rl, lf, rf, durationMs);
  buzzerStopTone();
  servoSmartDelay(20);
}


void silentMove(int durationMs, int ll, int rl, int lf, int rf) {
  moveServosSmooth(ll, rl, lf, rf, durationMs);
  servoSmartDelay(20);
}

// Efectos avanzados de sonido + movimiento
void toneMoveSweep(int startFreq, int endFreq, int durationMs, int ll, int rl, int lf, int rf) {
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
    int currentFreq = startFreq + ((endFreq - startFreq) * i) / steps;

    int newLL = startLL + ((ll - startLL) * i) / steps;
    int newRL = startRL + ((rl - startRL) * i) / steps;
    int newLF = startLF + ((lf - startLF) * i) / steps;
    int newRF = startRF + ((rf - startRF) * i) / steps;

    buzzerStartTone(currentFreq);
    writeServosRaw(newLL, newRL, newLF, newRF);
    servoSmartDelay(20);
  }

  buzzerStopTone();
  servoSmartDelay(20);
}


void toneMoveArcade(int f1, int f2, int f3, int durationMs, int ll, int rl, int lf, int rf) {
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


void toneMovePulse(int lowFreq, int highFreq, int durationMs, int ll, int rl, int lf, int rf) {
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
    if ((i / 3) % 2 == 0) {
      buzzerStartTone(lowFreq);
    } else {
      buzzerStartTone(highFreq);
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
    if ((i / 2) % 2 == 0) {
      buzzerStartTone(freq);
    } else {
      buzzerStopTone();
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



// Alias de efectos sonoros
void toneMoveDrum(int durationMs, int ll, int rl, int lf, int rf) {
  toneMovePulse(180, 420, durationMs, ll, rl, lf, rf);
}


void toneMoveLaser(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveSweep(600, 2100, durationMs, ll, rl, lf, rf);
}


void toneMoveLaserDown(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveSweep(2200, 500, durationMs, ll, rl, lf, rf);
}


void toneMoveCute(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveArcade(1200, 1700, 2200, durationMs, ll, rl, lf, rf);
}


void toneMoveSuspense(int durationMs, int ll, int rl, int lf, int rf) {
  toneMovePulse(650, 760, durationMs, ll, rl, lf, rf);
}


void toneMoveVictory(int durationMs, int ll, int rl, int lf, int rf) {
  toneMoveArcade(900, 1400, 1900, durationMs, ll, rl, lf, rf);
}

// Setup de servos
void setupServos() {
  Serial.println("SERVOS: inicializando...");

  // Conexion fisica real:
  //
  // SERVO_1_PIN -> pie derecho
  // SERVO_2_PIN -> pie izquierdo
  // SERVO_3_PIN -> pierna derecha
  // SERVO_4_PIN -> pierna izquierda
  //
  // Mapeo logico corregido:
  //
  // servo1 = pierna izquierda -> SERVO_4_PIN
  // servo2 = pierna derecha   -> SERVO_3_PIN
  // servo3 = pie izquierdo    -> SERVO_2_PIN
  // servo4 = pie derecho      -> SERVO_1_PIN

  servo1.attach(SERVO_4_PIN);
  servoSmartDelay(150);

  servo2.attach(SERVO_3_PIN);
  servoSmartDelay(150);

  servo3.attach(SERVO_2_PIN);
  servoSmartDelay(150);

  servo4.attach(SERVO_1_PIN);
  servoSmartDelay(150);

  writeServosRaw(90, 90, 90, 90);
  servoSmartDelay(500);

  randomSeed(analogRead(PIN_LDR) + millis());

  Serial.println("SERVOS: listos en centro con mapeo corregido");
}


void centerServosSoft() {
  moveServosSmooth(90, 90, 90, 90, 450);
}


void centerServosFast() {
  moveServosSmooth(90, 90, 90, 90, 220);
}


// SHOWS DE FERIA COMANDOS SERVIDOR

// SHOW 1: Escaneo robotico
// Sonido: laser, scanner, radar
// Movimiento: giro de caderas y pies tipo escaneo
void showRobotScanner() {
  Serial.println("SHOW: ROBOT_SCANNER");

  centerServosSoft();

  toneMoveLaser(280, 72, 108, 90, 90);
  toneMoveLaserDown(260, 78, 102, 84, 96);
  toneMovePulse(700, 1300, 260, 90, 90, 78, 102);
  toneMoveLaser(300, 102, 78, 84, 96);
  toneMoveLaserDown(320, 108, 72, 90, 90);

  toneMoveStutter(1450, 260, 102, 78, 96, 84);
  toneMoveStutter(1150, 260, 90, 90, 102, 78);
  toneMovePulse(900, 1700, 280, 78, 102, 96, 84);

  toneMoveLaser(320, 82, 98, 82, 98);
  toneMoveLaserDown(340, 98, 82, 98, 82);

  centerServosSoft();
}

// SHOW 2: Alarma cute
// Sonido: campanitas roboticas
// Movimiento: suave, amigable y llamativo
void showCuteRobotAlarm() {
  Serial.println("SHOW: CUTE_ROBOT_ALARM");

  centerServosSoft();

  toneMoveCute(260, 86, 94, 86, 94);
  toneMoveArcade(1500, 1900, 2300, 260, 94, 86, 94, 86);
  toneMoveArcade(1800, 2200, 2600, 300, 88, 92, 80, 100);

  toneMoveCute(260, 92, 88, 100, 80);
  toneMoveArcade(1300, 1700, 2100, 300, 90, 90, 86, 94);

  toneMoveArcade(1600, 2000, 2400, 260, 84, 96, 94, 86);
  toneMoveArcade(1700, 2100, 2500, 260, 96, 84, 86, 94);
  toneMoveCute(320, 90, 90, 78, 102);

  toneMoveArcade(2200, 1800, 1400, 320, 90, 90, 102, 78);
  toneMoveCute(300, 86, 94, 88, 92);
  toneMoveArcade(1800, 1400, 1000, 320, 94, 86, 92, 88);

  centerServosSoft();
}

// SHOW 3: Baile de victoria
// Sonido: fanfarria arcade
// Movimiento: celebracion
void showVictoryRobotDance() {
  Serial.println("SHOW: VICTORY_ROBOT_DANCE");

  centerServosSoft();

  toneMoveVictory(240, 78, 102, 84, 96);
  toneMoveArcade(1000, 1500, 2100, 240, 102, 78, 96, 84);
  toneMoveArcade(1200, 1800, 2400, 260, 78, 102, 96, 84);
  toneMoveArcade(1600, 2200, 2800, 320, 102, 78, 84, 96);

  toneMovePulse(900, 1900, 280, 74, 106, 90, 90);
  toneMovePulse(1100, 2100, 280, 106, 74, 90, 90);

  toneMoveVictory(320, 90, 90, 76, 104);

  toneMoveArcade(2500, 2000, 1500, 260, 82, 98, 76, 104);
  toneMoveArcade(2300, 1800, 1300, 260, 98, 82, 104, 76);

  toneMovePulse(1300, 2300, 340, 90, 90, 84, 96);
  toneMoveVictory(360, 90, 90, 104, 76);

  centerServosSoft();
}

// SHOW 4: Pataditas roboticas
// Sonido: golpes, impactos, stutter
// Movimiento: patadas pequenas alternadas
void showRobotKicks() {
  Serial.println("SHOW: ROBOT_KICKS");

  centerServosSoft();

  toneMoveDrum(280, 78, 104, 90, 90);
  toneMovePulse(220, 900, 320, 76, 106, 70, 100);
  toneMoveStutter(1500, 240, 82, 100, 82, 96);
  toneMoveDrum(240, 90, 90, 90, 90);

  toneMoveDrum(280, 104, 78, 90, 90);
  toneMovePulse(250, 1000, 320, 106, 76, 100, 70);
  toneMoveStutter(1700, 240, 100, 82, 96, 82);
  toneMoveDrum(240, 90, 90, 90, 90);

  toneMovePulse(280, 1300, 200, 78, 104, 72, 98);
  toneMoveStutter(1800, 180, 90, 90, 90, 90);
  toneMovePulse(300, 1500, 200, 104, 78, 98, 72);
  toneMoveStutter(1900, 180, 90, 90, 90, 90);

  toneMoveDrum(340, 90, 90, 82, 98);

  centerServosSoft();
}

// SHOW 5: Reverencia / saludo
// Sonido: elegante, ceremonial
// Movimiento: inclinacion lenta y regreso
void showRobotBow() {
  Serial.println("SHOW: ROBOT_BOW");

  centerServosSoft();

  toneMoveArcade(500, 800, 1100, 340, 90, 90, 104, 76);
  toneMoveArcade(600, 900, 1300, 380, 88, 92, 108, 72);
  toneMoveArcade(700, 1000, 1500, 440, 86, 94, 112, 68);

  servoSmartDelay(120);

  toneMoveSweep(1500, 900, 340, 84, 96, 104, 76);
  toneMoveSweep(1700, 1000, 340, 96, 84, 104, 76);

  servoSmartDelay(100);

  toneMoveVictory(420, 90, 90, 90, 90);

  toneMoveCute(280, 82, 98, 88, 92);
  toneMoveCute(280, 98, 82, 92, 88);

  centerServosSoft();
}


// SHOW 6: Pasos fuertes tipo robot
// Sonido: industrial, grave, mecanico
// Movimiento: stomp / pasos marcados
void showRobotStomp() {
  Serial.println("SHOW: ROBOT_STOMP");

  centerServosSoft();

  toneMoveDrum(280, 86, 94, 76, 104);
  toneMoveStutter(900, 190, 90, 90, 90, 90);

  toneMoveDrum(280, 94, 86, 104, 76);
  toneMoveStutter(1000, 190, 90, 90, 90, 90);

  toneMovePulse(220, 700, 300, 82, 98, 74, 106);
  toneMoveStutter(1100, 190, 90, 90, 90, 90);

  toneMovePulse(260, 800, 300, 98, 82, 106, 74);
  toneMoveStutter(1200, 190, 90, 90, 90, 90);

  toneMovePulse(300, 1500, 260, 80, 100, 80, 100);
  toneMovePulse(350, 1700, 260, 100, 80, 100, 80);
  toneMoveArcade(500, 1200, 2200, 360, 90, 90, 90, 90);

  centerServosSoft();
}


// SHOW 7: Pose de un solo pie
// Sonido: suspenso / equilibrio
// Movimiento: carga peso y levanta visualmente un pie
void showOneFootBalance() {
  Serial.println("SHOW: ONE_FOOT_BALANCE");

  centerServosSoft();

  toneMoveSuspense(280, 90, 90, 90, 90);

  // Carga peso hacia la derecha
  toneMoveSweep(700, 1100, 400, 74, 106, 88, 92);

  // Levanta visualmente el pie izquierdo
  toneMoveSweep(1100, 1700, 460, 70, 110, 66, 96);

  // Mantiene pose con pulso de suspenso
  toneMovePulse(800, 950, 560, 70, 110, 66, 96);

  // Pequeño balance
  toneMovePulse(850, 1300, 280, 74, 106, 70, 94);
  toneMovePulse(900, 1500, 280, 70, 110, 66, 96);

  // Baja lentamente
  toneMoveSweep(1600, 900, 420, 80, 100, 78, 94);
  toneMoveArcade(900, 1200, 1700, 440, 90, 90, 90, 90);

  // Mini pose contraria
  toneMoveSweep(700, 1200, 340, 106, 74, 92, 88);
  toneMoveSweep(1200, 1800, 400, 110, 70, 96, 66);

  toneMovePulse(850, 1050, 420, 110, 70, 96, 66);

  toneMoveArcade(1600, 1200, 800, 460, 90, 90, 90, 90);

  centerServosSoft();
}

// SHOW 8: Exhibicion completa
// Combina varios movimientos diferentes.
// Es largo y vistoso para feria.
void showFullExhibition() {
  Serial.println("SHOW: FULL_EXHIBITION");

  centerServosSoft();

  showRobotScanner();
  servoSmartDelay(200);

  showRobotStomp();
  servoSmartDelay(200);

  showRobotKicks();
  servoSmartDelay(200);

  showOneFootBalance();
  servoSmartDelay(200);

  showRobotBow();
  servoSmartDelay(200);

  showVictoryRobotDance();

  centerServosSoft();
}


// Seleccion random de shows
void playRandomShowcaseMove() {
  int option = random(0, 7);

  if (option == 0) {
    showRobotScanner();
  } else if (option == 1) {
    showCuteRobotAlarm();
  } else if (option == 2) {
    showVictoryRobotDance();
  } else if (option == 3) {
    showRobotKicks();
  } else if (option == 4) {
    showRobotBow();
  } else if (option == 5) {
    showRobotStomp();
  } else {
    showOneFootBalance();
  }
}

// Funciones llamadas por Reminders.ino
void reminderAgendaMovementStrong() {
  if (!reminderMotionAuthorized) {
    Serial.println("SERVOS: movimiento fuerte BLOQUEADO");
    return;
  }

  Serial.println("SERVOS: alarma fuerte feria variada");

  int option = random(0, 5);

  if (option == 0) {
    showVictoryRobotDance();
  } else if (option == 1) {
    showRobotKicks();
  } else if (option == 2) {
    showRobotScanner();
  } else if (option == 3) {
    showRobotStomp();
  } else {
    showOneFootBalance();
  }
}


void reminderAgendaMovementMedium() {
  if (!reminderMotionAuthorized) {
    Serial.println("SERVOS: movimiento medio BLOQUEADO");
    return;
  }

  Serial.println("SERVOS: alarma media feria variada");

  int option = random(0, 4);

  if (option == 0) {
    showCuteRobotAlarm();
  } else if (option == 1) {
    showRobotBow();
  } else if (option == 2) {
    showRobotScanner();
  } else {
    showRobotKicks();
  }
}


void reminderMovement() {
  reminderAgendaMovementMedium();
}

// Pruebas usadas por BLEModule
void testServos() {
  Serial.println("SERVOS: test individual");

  centerServosSoft();

  toneMove(900, 220, 80, 90, 90, 90);
  centerServosFast();

  toneMove(1100, 220, 90, 100, 90, 90);
  centerServosFast();

  toneMove(1300, 220, 90, 90, 80, 90);
  centerServosFast();

  toneMove(1500, 220, 90, 90, 90, 100);
  centerServosSoft();
}


void testWalkServos() {
  Serial.println("SERVOS: demo caminata/pasos");
  showRobotStomp();
}


void testWalkStronger() {
  Serial.println("SERVOS: demo fuerte feria");
  showFullExhibition();
}

// Comandos para BLE o Serial
// Devuelve true si el comando fue reconocido.

bool handleServoCommand(String type) {
  type.trim();

  if (type == "ALARM_TEST" || type == "SHOW_RANDOM") {
    playRandomShowcaseMove();
    return true;
  }

  if (type == "SHOW_SCAN") {
    showRobotScanner();
    return true;
  }

  if (type == "SHOW_CUTE") {
    showCuteRobotAlarm();
    return true;
  }

  if (type == "SHOW_WIN") {
    showVictoryRobotDance();
    return true;
  }

  if (type == "SHOW_KICK") {
    showRobotKicks();
    return true;
  }

  if (type == "SHOW_BOW") {
    showRobotBow();
    return true;
  }

  if (type == "SHOW_STOMP") {
    showRobotStomp();
    return true;
  }

  if (type == "SHOW_ONE_FOOT") {
    showOneFootBalance();
    return true;
  }

  if (type == "SHOW_FULL") {
    showFullExhibition();
    return true;
  }

  return false;
}