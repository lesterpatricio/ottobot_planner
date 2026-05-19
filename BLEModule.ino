// BLEModule.ino - OttoBot Agenda
// BLE + comandos del robot
// Integrado con logs WiFi 

String lastRobotResponse = "";

void sendRobotResponse(String response) {
  lastRobotResponse = response;
  responseChar.writeValue(response);
}

void setupBLE() {
  if (!BLE.begin()) {
    Serial.println("ERROR BLE");
    while (1);
  }

  BLE.setLocalName("OttoBot");
  BLE.setDeviceName("OttoBot");
  BLE.setAdvertisedService(ottoService);

  ottoService.addCharacteristic(commandChar);
  ottoService.addCharacteristic(responseChar);
  BLE.addService(ottoService);

  responseChar.writeValue("READY");
  BLE.advertise();

  Serial.println("BLE listo como OttoBot");
}

void handleCommand(String cmd) {
  cmd.trim();

  if (cmd.length() == 0) return;

  lastRobotResponse = "";

  logEvent("BLE command: " + cmd);

  // Log para dashboard. No altera la ejecución del comando.
  queueEventLogFull("COMMAND_RECEIVED", "Comando recibido por robot", cmd, "");

  String type = getPart(cmd, '|', 0);
  type.trim();

  if (type == "TEST") {
    sendRobotResponse("OK|TEST");
    queueEventLogFull("COMMAND_EXECUTED", "Comando TEST ejecutado", cmd, "OK|TEST");
    soundDefaultRobot();
    return;
  }

  if (type == "TEST_ALL") {
    sendRobotResponse("OK|TEST_ALL");
    queueEventLogFull("COMMAND_EXECUTED", "Comando TEST_ALL ejecutado", cmd, "OK|TEST_ALL");

    startCheckAnimation();
    soundReminder();
    testServos();

    return;
  }

  // Caminado
  if (type == "WALK") {
    sendRobotResponse("OK|WALK");
    queueEventLogFull("COMMAND_EXECUTED", "Comando WALK ejecutado", cmd, "OK|WALK");
    testWalkServos();
    return;
  }

  if (type == "WALK_STRONG") {
    sendRobotResponse("OK|WALK_STRONG");
    queueEventLogFull("COMMAND_EXECUTED", "Comando WALK_STRONG ejecutado", cmd, "OK|WALK_STRONG");
    testWalkStronger();
    return;
  }

  if (type == "TXT") {
    String msg = getPart(cmd, '|', 1);

    sendRobotResponse("OK|TXT");
    queueEventLogFull("TEXT_COMMAND", "Texto mostrado en robot", cmd, "OK|TXT");

    startScrollText(msg, false);
    return;
  }

  if (type == "REM") {
    String id = getPart(cmd, '|', 1);
    String date = getPart(cmd, '|', 2);
    String time = getPart(cmd, '|', 3);
    String msg = getPart(cmd, '|', 4);

    saveReminder(id, date, time, msg, false);

    // saveReminder ya responde OK o ERR.
    // Registramos comando recibido, no forzamos respuesta aquí.
    queueEventLogFull("REMINDER_COMMAND", "Solicitud de guardar recordatorio", cmd, lastRobotResponse);

    return;
  }

  if (type == "EDIT") {
    String id = getPart(cmd, '|', 1);
    String date = getPart(cmd, '|', 2);
    String time = getPart(cmd, '|', 3);
    String msg = getPart(cmd, '|', 4);

    saveReminder(id, date, time, msg, true);

    queueEventLogFull("REMINDER_EDIT_COMMAND", "Solicitud de editar recordatorio", cmd, lastRobotResponse);

    return;
  }

  if (type == "DEL") {
    String id = getPart(cmd, '|', 1);

    deleteReminder(id);

    queueEventLogFull("REMINDER_DELETE_COMMAND", "Solicitud de eliminar recordatorio", cmd, lastRobotResponse);

    return;
  }

  if (type == "CLR") {
    clearReminders();

    queueEventLogFull("REMINDER_CLEAR_COMMAND", "Solicitud de limpiar recordatorios", cmd, lastRobotResponse);

    return;
  }

  if (type == "LIST") {
    String list = "REMINDERS:";

    for (int i = 0; i < MAX_REMINDERS; i++) {
      if (reminders[i].active) {
        list += reminders[i].id + ",";
      }
    }

    sendRobotResponse(list);
    queueEventLogFull("REMINDER_LIST", "Listado de recordatorios solicitado", cmd, list);

    return;
  }

  // Comandos nuevos de feria: movimiento + buzzer sincronizado
  if (handleServoCommand(type)) {
    String resp = "OK|" + type;
    sendRobotResponse(resp);
    queueEventLogFull("SHOW_COMMAND", "Comando de movimiento/show ejecutado", cmd, resp);
    return;
  }

  sendRobotResponse("ERR|UNKNOWN_COMMAND");
  queueEventLogFull("UNKNOWN_COMMAND", "Comando desconocido", cmd, "ERR|UNKNOWN_COMMAND");
}

void updateBLE() {
  BLEDevice central = BLE.central();

  if (central) {
    logEvent("BLE connected: " + central.address());
    queueEventLog("BLE_CONNECTED", central.address());

    soundBluetoothConnected();

    while (central.connected()) {
      BLE.poll();

      if (commandChar.written()) {
        String cmd = commandChar.value();
        handleCommand(cmd);
      }

      updateLDR();
      updateTouch();
      checkReminders();
      updateStateMachine();

      // Importante:
      // Si el celular está conectado, loop() queda dentro de este while.
      // Por eso el WiFi también debe actualizarse aquí.
      updateWiFiEvents();
    }

    logEvent("BLE disconnected: " + central.address());
    queueEventLog("BLE_DISCONNECTED", central.address());
  }
}