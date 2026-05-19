// Display.ino - OttoBot Agenda
// OLED + RoboEyes + Reloj automatico
// Ojos: 10 segundos
// Reloj: 5 segundos

// CONFIGURACION DE DURACION OJOS / RELOJ


// DURACION DE LOS OJOS
// 10000 ms = 10 segundos
const unsigned long AUTO_EYES_DURATION_MS = 10000;

// DURACION DEL RELOJ
// 5000 ms = 5 segundos
const unsigned long AUTO_CLOCK_DURATION_MS = 5000;

// SETUP DISPLAY
void setupDisplay() {
  if (!display.begin(I2C_ADDRESS, true)) {
    Serial.println("ERROR OLED");
    while (1);
  }

  display.setTextWrap(false);
  display.clearDisplay();
  display.display();
}

// TEXTO
void printBoldText(String texto, int x, int y, int size) {
  display.setTextWrap(false);
  display.setTextSize(size);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(x, y);
  display.print(texto);

  display.setCursor(x + 1, y);
  display.print(texto);
}


void drawCenteredText(String text, int y, int size) {
  int16_t x1, y1;
  uint16_t w, h;

  display.setTextSize(size);
  display.setTextColor(SH110X_WHITE);
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.print(text);
}



// ROBOT UI BASE
void drawRobotFrame() {
  display.drawRoundRect(0, 0, 128, 64, 6, SH110X_WHITE);

  // Esquinas HUD
  display.drawLine(6, 6, 20, 6, SH110X_WHITE);
  display.drawLine(6, 6, 6, 20, SH110X_WHITE);

  display.drawLine(108, 6, 122, 6, SH110X_WHITE);
  display.drawLine(122, 6, 122, 20, SH110X_WHITE);

  display.drawLine(6, 58, 20, 58, SH110X_WHITE);
  display.drawLine(6, 44, 6, 58, SH110X_WHITE);

  display.drawLine(108, 58, 122, 58, SH110X_WHITE);
  display.drawLine(122, 44, 122, 58, SH110X_WHITE);
}


void drawRobotPanel(int x, int y, int w, int h) {
  display.drawRoundRect(x, y, w, h, 6, SH110X_WHITE);

  display.fillCircle(x + 6, y + 6, 1, SH110X_WHITE);
  display.fillCircle(x + w - 7, y + h - 7, 1, SH110X_WHITE);
}

// GUARDADO ROBOT UI
void drawLargeCheck() {
  display.clearDisplay();
  display.setTextWrap(false);

  drawRobotFrame();
  drawRobotPanel(38, 9, 52, 32);

  // Icono de Check
  display.drawLine(53, 25, 61, 33, SH110X_WHITE);
  display.drawLine(61, 33, 76, 17, SH110X_WHITE);

  drawCenteredText("GUARDADO", 50, 1);

  display.display();
}

// RECORDATORIO ROBOT UI
void drawNotificationBell() {
  display.clearDisplay();
  display.setTextWrap(false);

  drawRobotFrame();
  drawRobotPanel(38, 8, 52, 34);

  // Luz superior tipo alerta
  display.fillCircle(64, 14, 2, SH110X_WHITE);

  // Campana minimalista
  display.drawRoundRect(52, 19, 24, 16, 5, SH110X_WHITE);
  display.drawLine(50, 35, 78, 35, SH110X_WHITE);
  display.fillCircle(64, 38, 2, SH110X_WHITE);

  // Señales laterales
  display.drawLine(30, 22, 34, 18, SH110X_WHITE);
  display.drawLine(30, 28, 34, 28, SH110X_WHITE);
  display.drawLine(30, 34, 34, 38, SH110X_WHITE);

  display.drawLine(98, 22, 94, 18, SH110X_WHITE);
  display.drawLine(98, 28, 94, 28, SH110X_WHITE);
  display.drawLine(98, 34, 94, 38, SH110X_WHITE);

  drawCenteredText("RECORDATORIO", 50, 1);

  display.display();
}

// ANIMACION GUARDADO
void startCheckAnimation() {
  currentState = SHOW_CHECK;
  clockMode = false;
  checkStartTime = millis();

  display.clearDisplay();
  display.setTextWrap(false);

  drawRobotFrame();
  drawRobotPanel(38, 9, 52, 32);
  display.display();
  delay(100);

  display.drawLine(53, 25, 61, 33, SH110X_WHITE);
  display.display();
  delay(100);

  display.drawLine(61, 33, 76, 17, SH110X_WHITE);
  display.display();
  delay(100);

  drawCenteredText("GUARDADO", 50, 1);
  display.display();

  soundSuccess();
}

// TEXTO EN MOVIMIENTO
void startScrollText(String msg, bool afterReminder) {
  scrollMessage = normalizarTexto(msg);
  scrollX = SCREEN_WIDTH;
  currentState = SHOW_SCROLL_TEXT;

  scrollAfterReminder = afterReminder;
  clockMode = false;
  lastScrollUpdate = 0;
}


void updateScrollText() {
  if (millis() - lastScrollUpdate < SCROLL_SPEED_MS) return;
  lastScrollUpdate = millis();

  display.clearDisplay();
  display.setTextWrap(false);

  drawRobotFrame();

  int16_t x1, y1;
  uint16_t w, h;

  display.setTextSize(SCROLL_TEXT_SIZE);
  display.setTextColor(SH110X_WHITE);
  display.getTextBounds(scrollMessage, 0, 0, &x1, &y1, &w, &h);

  printBoldText(scrollMessage, scrollX, SCROLL_TEXT_Y, SCROLL_TEXT_SIZE);

  display.display();

  scrollX -= 2;

  if (scrollX < -((int)w + 20)) {
    if (scrollAfterReminder) {
      showPendingList();
    } else {
      enterStandby();
    }
  }
}



// RELOJ ROBOT UI
void drawDigitalClock() {
  DateTime now = rtc.now();

  char hora[6];
  sprintf(hora, "%02d:%02d", now.hour(), now.minute());

  char fecha[16];
  sprintf(fecha, "%02d/%02d/%04d", now.day(), now.month(), now.year());

  display.clearDisplay();
  display.setTextWrap(false);

  drawRobotFrame();

  int16_t x1, y1;
  uint16_t w, h;

  display.setTextSize(3);
  display.getTextBounds(hora, 0, 0, &x1, &y1, &w, &h);

  int xHora = (SCREEN_WIDTH - w) / 2;
  printBoldText(String(hora), xHora, 17, 3);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.getTextBounds(fecha, 0, 0, &x1, &y1, &w, &h);

  display.setCursor((SCREEN_WIDTH - w) / 2, 51);
  display.print(fecha);

  display.display();
}

// LISTA DE PENDIENTES ROBOT UI
void showPendingList() {
  currentState = SHOW_PENDING_LIST;
  pendingListStart = millis();
  scrollAfterReminder = false;

  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(SH110X_WHITE);

  drawRobotFrame();

  display.setTextSize(1);
  drawCenteredText("PENDIENTES", 6, 1);

  display.drawLine(20, 17, 108, 17, SH110X_WHITE);

  DateTime now = rtc.now();

  int lineY = 22;
  int count = 1;

  for (int i = 0; i < MAX_REMINDERS; i++) {
    if (!reminders[i].active) continue;

    bool futureDate =
      reminders[i].year > now.year() ||
      (reminders[i].year == now.year() && reminders[i].month > now.month()) ||
      (reminders[i].year == now.year() && reminders[i].month == now.month() && reminders[i].day > now.day());

    bool sameDayFutureTime =
      reminders[i].year == now.year() &&
      reminders[i].month == now.month() &&
      reminders[i].day == now.day() &&
      (
        reminders[i].hour > now.hour() ||
        (reminders[i].hour == now.hour() && reminders[i].minute > now.minute())
      );

    if (futureDate || sameDayFutureTime) {
      display.setCursor(10, lineY);

      display.print(count);
      display.print("> ");

      if (reminders[i].day < 10) display.print("0");
      display.print(reminders[i].day);
      display.print("/");

      if (reminders[i].month < 10) display.print("0");
      display.print(reminders[i].month);

      display.print(" ");

      if (reminders[i].hour < 10) display.print("0");
      display.print(reminders[i].hour);
      display.print(":");

      if (reminders[i].minute < 10) display.print("0");
      display.print(reminders[i].minute);

      display.print(" OK");

      lineY += 10;
      count++;

      if (lineY > 54) break;
    }
  }

  if (count == 1) {
    drawCenteredText("SIN PENDIENTES", 32, 1);
  }

  display.display();
}


void updatePendingList() {
  if (millis() - pendingListStart >= PENDING_LIST_DURATION) {
    enterStandby();
  }
}


// STANDBY AUTOMATICO
// Intercambia entre ojos y reloj.
// Ojos: AUTO_EYES_DURATION_MS
// Reloj: AUTO_CLOCK_DURATION_MS

void updateStandbyMode() {
  unsigned long now = millis();
  // 1. Si esta mostrando ojos, despues de 10 segundos
  //    cambia automaticamente al reloj.

  if (!clockMode && now - standbyTimer >= AUTO_EYES_DURATION_MS) {
    clockMode = true;
    standbyTimer = now;
    lastClockDraw = 0;

    display.clearDisplay();
    display.display();

   // Serial.println("DISPLAY: cambio automatico a RELOJ");
  }


  // 2. Si esta mostrando reloj, despues de 5 segundos
  //    regresa automaticamente a los ojos.
  if (clockMode && now - standbyTimer >= AUTO_CLOCK_DURATION_MS) {
    clockMode = false;
    standbyTimer = now;

    display.clearDisplay();
    display.display();

    //Serial.println("DISPLAY: cambio automatico a OJOS");
  }

  // 3. Actualizar pantalla segun el modo actual

  if (clockMode) {
    // El reloj se redibuja cada 1 segundo
    if (now - lastClockDraw >= 1000) {
      lastClockDraw = now;
      drawDigitalClock();
    }
  } else {
    // Los ojos siguen funcionando normal
    applyLdrBehavior();
    roboEyes.update();
  }
}