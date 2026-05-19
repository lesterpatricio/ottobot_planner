// WiFiEvents.ino - OttoBot Agenda
// Logs + comandos desde dashboard web

// Backend UMG:
//   POST /events
//   GET  /commands/pending?deviceId=arduino_nano_33_01
//   POST /commands/:id/ack
// Los comandos WiFi se ejecutan usando handleCommand(commandText),


#include <WiFiNINA.h>


// CONFIGURACION WIFI

// Cambia estos datos por la red real del robot
char WIFI_SSID[] = "WIFINAME";
char WIFI_PASS[] = "PASSWORD";

// Servidor público del proyecto.
// IMPORTANTE: no poner "http://"
char SERVER_HOST[] = "IP";

const int SERVER_PORT = PORT;

const char EVENTS_PATH[] = "/events";
const char COMMANDS_PENDING_PATH[] = "/commands/pending";

// Debe coincidir con el dashboard
String DEVICE_ID = "arduino_nano_33_01";


// TIEMPOS
const unsigned long WIFI_SEND_INTERVAL = 5000;
const unsigned long WIFI_COMMAND_INTERVAL = 3000;
const unsigned long WIFI_RECONNECT_INTERVAL = 15000;

const unsigned long HTTP_TIMEOUT_MS = 1200;
const unsigned long WIFI_STARTUP_TIMEOUT_MS = 3000;

// COLA DE LOGS
#define MAX_EVENT_QUEUE 10

struct PendingEvent {
  String actionType;
  String message;
  String command;
  String robotResponse;
};

PendingEvent eventQueue[MAX_EVENT_QUEUE];
int eventQueueCount = 0;

// VARIABLES INTERNAS
unsigned long lastWiFiSend = 0;
unsigned long lastWiFiCommandCheck = 0;
unsigned long lastWiFiReconnect = 0;

bool wifiCommandRunning = false;

WiFiClient eventClient;

// Esta variable viene de BLEModule.ino
extern String lastRobotResponse;

// UTILIDADES
String escapeJson(String text) {
  text.replace("\\", "\\\\");
  text.replace("\"", "\\\"");
  text.replace("\n", " ");
  text.replace("\r", " ");
  text.replace("\t", " ");
  return text;
}

String urlEncodeLite(String text) {
  text.replace(" ", "%20");
  text.replace("|", "%7C");
  text.replace(":", "%3A");
  text.replace("/", "%2F");
  return text;
}

String wifiStatusText() {
  int status = WiFi.status();

  if (status == WL_CONNECTED) return "WL_CONNECTED";
  if (status == WL_IDLE_STATUS) return "WL_IDLE_STATUS";
  if (status == WL_NO_SSID_AVAIL) return "WL_NO_SSID_AVAIL";
  if (status == WL_CONNECT_FAILED) return "WL_CONNECT_FAILED";
  if (status == WL_CONNECTION_LOST) return "WL_CONNECTION_LOST";
  if (status == WL_DISCONNECTED) return "WL_DISCONNECTED";

  return "UNKNOWN";
}

String extractJsonString(String json, String key) {
  String pattern = "\"" + key + "\"";

  int keyIndex = json.indexOf(pattern);
  if (keyIndex < 0) return "";

  int colonIndex = json.indexOf(":", keyIndex);
  if (colonIndex < 0) return "";

  int firstQuote = json.indexOf("\"", colonIndex + 1);
  if (firstQuote < 0) return "";

  int secondQuote = firstQuote + 1;

  while (secondQuote < json.length()) {
    if (json.charAt(secondQuote) == '"' && json.charAt(secondQuote - 1) != '\\') {
      break;
    }

    secondQuote++;
  }

  if (secondQuote >= json.length()) return "";

  String value = json.substring(firstQuote + 1, secondQuote);

  value.replace("\\\"", "\"");
  value.replace("\\\\", "\\");

  return value;
}

bool extractJsonBool(String json, String key) {
  String pattern = "\"" + key + "\"";

  int keyIndex = json.indexOf(pattern);
  if (keyIndex < 0) return false;

  int colonIndex = json.indexOf(":", keyIndex);
  if (colonIndex < 0) return false;

  String rest = json.substring(colonIndex + 1);
  rest.trim();

  return rest.startsWith("true");
}

// JSON DE EVENTOS

String buildEventJson(String actionType, String message, String command, String robotResponse) {
  String json = "{";

  json += "\"actionType\":\"" + escapeJson(actionType) + "\",";
  json += "\"deviceId\":\"" + escapeJson(DEVICE_ID) + "\",";
  json += "\"source\":\"robot\",";
  json += "\"channel\":\"wifi\",";
  json += "\"status\":\"executed\",";
  json += "\"message\":\"" + escapeJson(message) + "\"";

  if (command.length() > 0) {
    json += ",\"command\":\"" + escapeJson(command) + "\"";
  }

  if (robotResponse.length() > 0) {
    json += ",\"robotResponse\":\"" + escapeJson(robotResponse) + "\"";
  }

  json += ",\"uptimeMs\":" + String(millis());

  json += "}";

  return json;
}

// SETUP WIFI
void setupWiFiEvents() {
  Serial.println("WIFI: inicializando modulo de eventos/comandos...");

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("WIFI ERROR: modulo NINA no detectado");
    return;
  }

  WiFi.end();
  delay(150);

  Serial.print("WIFI: conectando a ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_STARTUP_TIMEOUT_MS) {
    Serial.print(".");
    BLE.poll();
    delay(250);
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WIFI: conectado correctamente");

    Serial.print("WIFI: IP local: ");
    Serial.println(WiFi.localIP());

    Serial.print("WIFI: RSSI: ");
    Serial.println(WiFi.RSSI());

    queueEventLog("WIFI_CONNECTED", "Robot conectado a WiFi");
  } else {
    Serial.print("WIFI: no conectado al inicio. Estado: ");
    Serial.println(wifiStatusText());
    Serial.println("WIFI: se reintentara en segundo plano");
  }
}

// COLA DE EVENTOS
void queueEventLogFull(String actionType, String message, String command, String robotResponse) {
  actionType.trim();
  message.trim();
  command.trim();
  robotResponse.trim();

  if (actionType.length() == 0) actionType = "ROBOT_LOG";
  if (message.length() == 0) message = "Evento sin descripcion";

  if (eventQueueCount >= MAX_EVENT_QUEUE) {
    for (int i = 1; i < MAX_EVENT_QUEUE; i++) {
      eventQueue[i - 1] = eventQueue[i];
    }

    eventQueueCount = MAX_EVENT_QUEUE - 1;
  }

  eventQueue[eventQueueCount].actionType = actionType;
  eventQueue[eventQueueCount].message = message;
  eventQueue[eventQueueCount].command = command;
  eventQueue[eventQueueCount].robotResponse = robotResponse;

  eventQueueCount++;

  Serial.print("WIFI LOG ENCOLADO: ");
  Serial.print(actionType);
  Serial.print(" | ");
  Serial.println(message);
}

void queueEventLog(String actionType, String message) {
  queueEventLogFull(actionType, message, "", "");
}

void removeFirstQueuedEvent() {
  if (eventQueueCount <= 0) return;

  for (int i = 0; i < eventQueueCount - 1; i++) {
    eventQueue[i] = eventQueue[i + 1];
  }

  eventQueueCount--;
}


// WIFI RECONNECT
void reconnectWiFiEventsIfNeeded() {
  if (WiFi.status() == WL_CONNECTED) return;

  if (millis() - lastWiFiReconnect < WIFI_RECONNECT_INTERVAL) return;
  lastWiFiReconnect = millis();

  Serial.print("WIFI: desconectado. Estado: ");
  Serial.println(wifiStatusText());

  Serial.println("WIFI: reintentando conexion...");

  WiFi.end();
  delay(100);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

// HTTP HELPERS
String readHttpResponse() {
  String response = "";
  unsigned long start = millis();

  while (millis() - start < HTTP_TIMEOUT_MS) {
    while (eventClient.available()) {
      char c = eventClient.read();
      response += c;

      if (response.length() > 1200) break;
    }

    if (!eventClient.connected()) break;
    if (response.length() > 1200) break;

    BLE.poll();
    delay(4);
  }

  return response;
}

int getHttpStatusCode(String response) {
  int httpIndex = response.indexOf("HTTP/");
  if (httpIndex < 0) return -1;

  int firstSpace = response.indexOf(' ', httpIndex);
  if (firstSpace < 0) return -1;

  String codeStr = response.substring(firstSpace + 1, firstSpace + 4);
  codeStr.trim();

  return codeStr.toInt();
}

String getHttpBody(String response) {
  int bodyIndex = response.indexOf("\r\n\r\n");

  if (bodyIndex >= 0) {
    return response.substring(bodyIndex + 4);
  }

  return "";
}

bool httpPost(String path, String jsonBody, int &statusCode, String &body) {
  statusCode = -1;
  body = "";

  if (WiFi.status() != WL_CONNECTED) return false;

  eventClient.stop();

  if (!eventClient.connect(SERVER_HOST, SERVER_PORT)) {
    Serial.println("HTTP POST: no conecta al backend");
    eventClient.stop();
    return false;
  }

  eventClient.print("POST ");
  eventClient.print(path);
  eventClient.println(" HTTP/1.1");

  eventClient.print("Host: ");
  eventClient.println(SERVER_HOST);

  eventClient.println("Content-Type: application/json");

  eventClient.print("Content-Length: ");
  eventClient.println(jsonBody.length());

  eventClient.println("Connection: close");
  eventClient.println();

  eventClient.print(jsonBody);

  String response = readHttpResponse();

  eventClient.stop();

  statusCode = getHttpStatusCode(response);
  body = getHttpBody(response);

  return statusCode > 0;
}

bool httpGet(String path, int &statusCode, String &body) {
  statusCode = -1;
  body = "";

  if (WiFi.status() != WL_CONNECTED) return false;

  eventClient.stop();

  if (!eventClient.connect(SERVER_HOST, SERVER_PORT)) {
    Serial.println("HTTP GET: no conecta al backend");
    eventClient.stop();
    return false;
  }

  eventClient.print("GET ");
  eventClient.print(path);
  eventClient.println(" HTTP/1.1");

  eventClient.print("Host: ");
  eventClient.println(SERVER_HOST);

  eventClient.println("Connection: close");
  eventClient.println();

  String response = readHttpResponse();

  eventClient.stop();

  statusCode = getHttpStatusCode(response);
  body = getHttpBody(response);

  return statusCode > 0;
}

// ENVIAR EVENTOS

bool sendEventToServer(String actionType, String message, String command, String robotResponse) {
  String json = buildEventJson(actionType, message, command, robotResponse);

  int statusCode = -1;
  String body = "";

  Serial.println("WIFI: enviando evento a /events");
  Serial.println(json);

  bool ok = httpPost(EVENTS_PATH, json, statusCode, body);

  Serial.print("WIFI EVENT status: ");
  Serial.println(statusCode);

  if (body.length() > 0) {
    Serial.print("WIFI EVENT body: ");
    Serial.println(body);
  }

  if (!ok) return false;

  if (statusCode == 200 || statusCode == 201 || statusCode == 202) {
    return true;
  }

  if (statusCode == 400) {
    Serial.println("WIFI EVENT: 400, evento invalido, se descarta");
    return true;
  }

  return false;
}

void sendOneQueuedEvent() {
  if (eventQueueCount <= 0) return;

  bool sent = sendEventToServer(
    eventQueue[0].actionType,
    eventQueue[0].message,
    eventQueue[0].command,
    eventQueue[0].robotResponse
  );

  if (sent) {
    Serial.print("WIFI LOG enviado: ");
    Serial.println(eventQueue[0].actionType);
    removeFirstQueuedEvent();
  } else {
    Serial.println("WIFI LOG no enviado, se reintentara luego");
  }
}

// COMANDOS DESDE DASHBOARD
String buildAckPath(String commandId) {
  return String("/commands/") + commandId + "/ack";
}

void ackCommandToServer(String commandId, String statusText, String robotResponse, String notes) {
  if (commandId.length() == 0) {
    Serial.println("WIFI ACK: commandId vacio, no se envia ACK");
    return;
  }

  String json = "{";
  json += "\"status\":\"" + escapeJson(statusText) + "\",";
  json += "\"robotResponse\":\"" + escapeJson(robotResponse) + "\",";
  json += "\"executionNotes\":\"" + escapeJson(notes) + "\",";
  json += "\"source\":\"arduino_wifi\"";
  json += "}";

  String path = buildAckPath(commandId);

  int statusCode = -1;
  String body = "";

  Serial.print("WIFI ACK path: ");
  Serial.println(path);

  bool ok = httpPost(path, json, statusCode, body);

  Serial.print("WIFI ACK status: ");
  Serial.println(statusCode);

  if (body.length() > 0) {
    Serial.print("WIFI ACK body: ");
    Serial.println(body);
  }

  if (!ok) {
    Serial.println("WIFI ACK: fallo HTTP");
  }
}

String getFirstAvailableCommandText(String body) {
  String cmd = extractJsonString(body, "commandText");

  if (cmd.length() > 0) return cmd;

  cmd = extractJsonString(body, "payload");

  if (cmd.length() > 0) return cmd;

  cmd = extractJsonString(body, "command");

  return cmd;
}

String getFirstAvailableCommandId(String body) {
  String id = extractJsonString(body, "_id");

  if (id.length() > 0) return id;

  id = extractJsonString(body, "id");

  if (id.length() > 0) return id;

  id = extractJsonString(body, "commandId");

  return id;
}

void checkPendingWifiCommand() {
  if (wifiCommandRunning) return;

  String path = String(COMMANDS_PENDING_PATH) + "?deviceId=" + urlEncodeLite(DEVICE_ID);

  int statusCode = -1;
  String body = "";

  Serial.print("WIFI CMD GET ");
  Serial.println(path);

  bool ok = httpGet(path, statusCode, body);

  Serial.print("WIFI CMD status: ");
  Serial.println(statusCode);

  if (!ok) return;
  if (statusCode != 200) return;

body.trim();

if (body.length() == 0) {
  Serial.println("WIFI CMD: body vacio");
  return;
}

// El backend devuelve [] cuando no hay comandos pendientes
if (body == "[]") {
  Serial.println("WIFI CMD: no hay comandos pendientes");
  return;
}

Serial.print("WIFI CMD body: ");
Serial.println(body);

if (body.indexOf("hasCommand") >= 0 && !extractJsonBool(body, "hasCommand")) {
  Serial.println("WIFI CMD: no hay comandos pendientes");
  return;
}

  String commandText = getFirstAvailableCommandText(body);
  String commandId = getFirstAvailableCommandId(body);

  if (commandText.length() == 0) {
    Serial.println("WIFI CMD: no se encontro commandText/payload/command");
    return;
  }

  Serial.print("WIFI CMD recibido: ");
  Serial.println(commandText);

  queueEventLogFull(
    "WIFI_COMMAND_RECEIVED",
    "Comando recibido desde dashboard",
    commandText,
    ""
  );

  wifiCommandRunning = true;

  // Ejecuta usando la misma lógica de BLE.
  handleCommand(commandText);

  wifiCommandRunning = false;

  String robotResponse = lastRobotResponse;

  if (robotResponse.length() == 0) {
    robotResponse = "OK|EXECUTED_BY_ARDUINO_WIFI";
  }

  ackCommandToServer(
    commandId,
    "executed",
    robotResponse,
    "Comando ejecutado directamente por Arduino via WiFi"
  );

  queueEventLogFull(
    "WIFI_COMMAND_EXECUTED",
    "Comando ejecutado desde dashboard",
    commandText,
    robotResponse
  );
}

// UPDATE PRINCIPAL WIFI
void updateWiFiEvents() {
  if (robotBusy) return;

  // Evita HTTP durante estados visuales sensibles.
  // Esto corrige los micro-bloqueos del scroll.
  if (currentState == REMINDER_ALERT) return;
  if (currentState == SHOW_SCROLL_TEXT) return;
  if (currentState == SHOW_CHECK) return;

  reconnectWiFiEventsIfNeeded();

  if (WiFi.status() != WL_CONNECTED) return;

  if (millis() - lastWiFiSend >= WIFI_SEND_INTERVAL) {
    lastWiFiSend = millis();
    sendOneQueuedEvent();
  }

  if (millis() - lastWiFiCommandCheck >= WIFI_COMMAND_INTERVAL) {
    lastWiFiCommandCheck = millis();
    checkPendingWifiCommand();
  }
}

// PRUEBA OPCIONAL TEST_WIFI
void testWiFiEvent() {
  queueEventLogFull(
    "TEST_WIFI",
    "Prueba desde Arduino Nano 33 IoT",
    "TEST",
    "OK|TEST_WIFI"
  );
}