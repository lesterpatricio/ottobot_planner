//Normalizar texto para AdaFruit
String normalizarTexto(String texto) {
  texto.replace("\n", " ");
  texto.replace("\r", " ");

  texto.replace("á", "A");
  texto.replace("é", "E");
  texto.replace("í", "I");
  texto.replace("ó", "O");
  texto.replace("ú", "U");
  texto.replace("Á", "A");
  texto.replace("É", "E");
  texto.replace("Í", "I");
  texto.replace("Ó", "O");
  texto.replace("Ú", "U");

  texto.replace("ñ", "N");
  texto.replace("Ñ", "N");
  texto.replace("¿", "");
  texto.replace("¡", "");

  texto.toUpperCase();
  return texto;
}

String getPart(String data, char separator, int index) {
  int found = 0;
  int start = 0;

  for (int i = 0; i <= data.length(); i++) {
    if (data.charAt(i) == separator || i == data.length()) {
      if (found == index) {
        return data.substring(start, i);
      }
      found++;
      start = i + 1;
    }
  }

  return "";
}

//Comunicacion USB
void updateSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() > 0) {
      Serial.print("Comando Serial recibido: ");
      Serial.println(cmd);
      handleCommand(cmd);
    }
  }
}

bool parseDate(String date, int &year, int &month, int &day) {
  if (date.length() != 10) return false;

  year = date.substring(0, 4).toInt();
  month = date.substring(5, 7).toInt();
  day = date.substring(8, 10).toInt();

  return year > 2020 && month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

bool parseTime(String time, int &hour, int &minute) {
  if (time.length() != 5) return false;

  hour = time.substring(0, 2).toInt();
  minute = time.substring(3, 5).toInt();

  return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

void logEvent(String action) {
  DateTime now = rtc.now();

  String logLine = "[";
  logLine += String(now.year()) + "-";
  logLine += String(now.month()) + "-";
  logLine += String(now.day()) + " ";
  logLine += String(now.hour()) + ":";
  logLine += String(now.minute()) + ":";
  logLine += String(now.second()) + "] ";
  logLine += action;

  Serial.println(logLine);
}