
// Sensors.ino - OttoBot Agenda
// LDR + Touch


// Touch:
// Toque largo >= 4 segundos cambia estado de animo.
// LDR:
//   1. OSCURIDAD
//   2. LUZ ALTA

// Oscuridad:
// - Ojos TIRED.
// - Recordatorios sin servos.
// - Recordatorios sin buzzer.

// LDR
void updateLDR() {
  ldrValue = analogRead(PIN_LDR);

  if (ldrValue < 100) {
    nightMode = true;
  } 
  else {
    nightMode = false;
  }
}

// MODO DE LUZ PARA RECORDATORIOS
LightMode getLightMode() {
  ldrValue = analogRead(PIN_LDR);

  Serial.print("LDR value: ");
  Serial.println(ldrValue);

  if (ldrValue < 100) {
    nightMode = true;
    return LIGHT_DARK;
  }

  nightMode = false;
  return LIGHT_HIGH;
}

// COMPORTAMIENTO AUTOMATICO POR LDR
void applyLdrBehavior() {
  if (currentState != STANDBY) return;

  updateLDR();

  if (nightMode) {
    roboEyes.setMood(TIRED);
  } 
  else {
    applyCurrentManualMood();
  }
}

// APLICAR MOOD MANUAL ACTUAL
void applyCurrentManualMood() {
  if (currentMoodIndex == 0) {
    roboEyes.setMood(DEFAULT);
  }

  else if (currentMoodIndex == 1) {
    roboEyes.setMood(HAPPY);
  }

  else if (currentMoodIndex == 2) {
    roboEyes.setMood(TIRED);
  }

  else if (currentMoodIndex == 3) {
    roboEyes.setMood(ANGRY);
  }
}


// TOUCH SENSOR - TOQUE LARGO

const unsigned long TOUCH_LONG_TIME = 4000;

bool touchWasPressed = false;
bool longTouchExecuted = false;

// TOUCH PRINCIPAL

void updateTouch() {
  static unsigned long highStartTime = 0;
  static bool touchConfirmed = false;
  
  // BLOQUEO DURANTE RECORDATORIO

  // Si hay recordatorio activo, sonido/movimiento/texto,
  // el touch no puede cambiar el ánimo ni interrumpir nada.
  if (currentState == REMINDER_ALERT || currentState == SHOW_SCROLL_TEXT) {
    touchWasPressed = false;
    longTouchExecuted = false;
    touchConfirmed = false;
    highStartTime = 0;
    return;
  }

  int touchState = digitalRead(PIN_TOUCH);
  unsigned long now = millis();

  // 1. Detectar posible toque
  if (touchState == HIGH && !touchConfirmed) {
    if (highStartTime == 0) {
      highStartTime = now;
    }

    // El HIGH debe mantenerse un poco para ser toque real.
    // Esto ignora toques fantasma.
    if (now - highStartTime >= 180) {
      touchConfirmed = true;
      touchWasPressed = true;
      longTouchExecuted = false;
      touchStartTime = now;

      Serial.println("TOUCH: inicio real");
    }
  }

  // Si volvió a LOW antes de confirmarse, era ruido
  if (touchState == LOW && !touchConfirmed) {
    highStartTime = 0;
  }

  // 2. Toque mantenido real
  if (touchState == HIGH && touchConfirmed && touchWasPressed) {
    unsigned long duration = now - touchStartTime;

    if (duration >= TOUCH_LONG_TIME && !longTouchExecuted) {
      longTouchExecuted = true;

      Serial.println("TOUCH: largo detectado");

      changeMoodByPet();
    }
  }

  // 3. Soltar touch
  if (touchState == LOW && touchConfirmed) {
    touchWasPressed = false;
    longTouchExecuted = false;
    touchConfirmed = false;
    highStartTime = 0;

    Serial.println("TOUCH: soltado");
  }
}

// CAMBIAR ESTADO / EMOCION CON TOQUE LARGO

void changeMoodByPet() {
  currentMoodIndex++;

  if (currentMoodIndex > 3) {
    currentMoodIndex = 0;
  }

  clockMode = false;
  currentState = STANDBY;
  standbyTimer = millis();

  display.clearDisplay();
  display.display();

  // Revisar LDR justo antes de permitir sonido
  updateLDR();

  // Si esta oscuro, el LDR tiene prioridad.
  // Guarda el mood manual, pero muestra TIRED y no suena.
  if (nightMode) {
    roboEyes.setMood(TIRED);
    buzzerStopTone();

    logEvent("TOUCH LONG: Oscuridad, mood TIRED sin sonido");
    return;
  }

  if (currentMoodIndex == 0) {
    roboEyes.setMood(DEFAULT);
    soundDefaultRobot();
    logEvent("TOUCH LONG: Mood DEFAULT");
  }

  else if (currentMoodIndex == 1) {
    roboEyes.setMood(HAPPY);
    soundHappyRobot();
    logEvent("TOUCH LONG: Mood HAPPY");
  }

  else if (currentMoodIndex == 2) {
    roboEyes.setMood(TIRED);
    soundTiredRobot();
    logEvent("TOUCH LONG: Mood TIRED");
  }

  else if (currentMoodIndex == 3) {
    roboEyes.setMood(ANGRY);
    soundAngryRobot();
    logEvent("TOUCH LONG: Mood ANGRY");
  }
}