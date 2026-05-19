
// Reminders.ino - OttoBot Agenda
// Gestión de recordatorios + activación de movimiento

// Esta variable está definida realmente en servos.ino.
// Aquí solo le avisamos al compilador que existe.
extern bool reminderMotionAuthorized;

int findReminderById(String id) {
  for (int i = 0; i < MAX_REMINDERS; i++) {
    if (reminders[i].active && reminders[i].id == id) return i;
  }
  return -1;
}

int findFreeReminderSlot() {
  for (int i = 0; i < MAX_REMINDERS; i++) {
    if (!reminders[i].active) return i;
  }
  return -1;
}

void saveReminder(String id, String date, String time, String msg, bool editMode) {
  int year, month, day, hour, minute;

  msg = normalizarTexto(msg);

  if (!parseDate(date, year, month, day) || !parseTime(time, hour, minute)) {
    responseChar.writeValue("ERR|FORMATO_FECHA_HORA");
    logEvent("ERROR: formato recordatorio");
    return;
  }

  int index = findReminderById(id);

  if (editMode) {
    if (index == -1) {
      responseChar.writeValue("ERR|ID_NO_EXISTE");
      return;
    }
  } else {
    if (index == -1) index = findFreeReminderSlot();
    if (index == -1) {
      responseChar.writeValue("ERR|SIN_ESPACIO");
      return;
    }
  }

  reminders[index].id = id;
  reminders[index].year = year;
  reminders[index].month = month;
  reminders[index].day = day;
  reminders[index].hour = hour;
  reminders[index].minute = minute;
  reminders[index].message = msg;
  reminders[index].active = true;
  reminders[index].triggeredToday = false;

  responseChar.writeValue("OK|REMINDER_SAVED");
  logEvent("REMINDER saved/edited ID=" + id + " MSG=" + msg);

  startCheckAnimation();
}

void deleteReminder(String id) {
  int index = findReminderById(id);

  if (index == -1) {
    responseChar.writeValue("ERR|ID_NO_EXISTE");
    return;
  }

  reminders[index].active = false;
  responseChar.writeValue("OK|DELETED");
  logEvent("REMINDER deleted ID=" + id);
}

void clearReminders() {
  for (int i = 0; i < MAX_REMINDERS; i++) {
    reminders[i].active = false;
  }

  responseChar.writeValue("OK|CLEARED");
  logEvent("REMINDER clear all");
}

void triggerReminder(Reminder r) {
  currentState = REMINDER_ALERT;
  clockMode = false;
  standbyTimer = millis();

  logEvent("REMINDER triggered ID=" + r.id + " MSG=" + r.message);

  LightMode mode = getLightMode();

  if (mode == LIGHT_DARK) {
    logEvent("LIGHT MODE: DARK - sin servos ni buzzer");

    roboEyes.setMood(TIRED);

    // Seguridad total: sin movimiento y sin sonido
    reminderMotionAuthorized = false;
    buzzerStopTone();

    // Solo muestra el texto del recordatorio
    startScrollText(r.message, true);
  }

  else {
    logEvent("LIGHT MODE: HIGH");

    roboEyes.setMood(HAPPY);

    drawNotificationBell();

    // Con luz: movimiento + sonido permitidos
    reminderMotionAuthorized = true;
    reminderAgendaMovementStrong();
    reminderMotionAuthorized = false;

    startScrollText(r.message, true);
  }

  responseChar.writeValue("OK|REMINDER_TRIGGERED|" + r.id);
}

void checkReminders() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 1000) return;
  lastCheck = millis();

  DateTime now = rtc.now();

  for (int i = 0; i < MAX_REMINDERS; i++) {
    if (!reminders[i].active) continue;

    bool sameDate =
      now.year() == reminders[i].year &&
      now.month() == reminders[i].month &&
      now.day() == reminders[i].day;

    bool sameTime =
      now.hour() == reminders[i].hour &&
      now.minute() == reminders[i].minute;

    if (sameDate && sameTime && !reminders[i].triggeredToday) {
      reminders[i].triggeredToday = true;
      triggerReminder(reminders[i]);
    }
  }
}