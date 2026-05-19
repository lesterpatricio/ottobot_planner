# OttoBot Agenda

OttoBot Agenda es un robot asistente basado en **Arduino Nano 33 IoT**. Su función principal es trabajar como una agenda física interactiva: recibe recordatorios desde una **app Flutter**, los guarda, los muestra en una pantalla OLED y reacciona con expresiones, sonido y movimiento.

El robot también se comunica con un **servidor por WiFi**, desde donde puede recibir comandos remotos para ejecutar animaciones y movimientos especiales.

---

## Funcionamiento principal

La agenda del robot se maneja principalmente desde la **aplicación Flutter** mediante Bluetooth Low Energy, BLE.

Desde la app se pueden realizar acciones como:

- Crear recordatorios.
- Editar recordatorios.
- Eliminar recordatorios.
- Limpiar recordatorios.
- Enviar texto al robot.
- Probar la comunicación con OttoBot.

Cuando llega la fecha y hora de un recordatorio, OttoBot:

1. Revisa la hora usando el módulo RTC DS3231.
2. Lee el nivel de luz con el sensor LDR.
3. Muestra el mensaje en la pantalla OLED.
4. Decide si debe sonar y moverse según la luz del ambiente.

---

## Comportamiento con el LDR

El sensor LDR permite que el robot adapte su comportamiento al ambiente.

### Con luz

Si hay luz suficiente:

- Muestra una animación de notificación.
- Activa el buzzer.
- Mueve los servos.
- Muestra el mensaje del recordatorio.

### En oscuridad

Si el ambiente está oscuro:

- Cambia a modo cansado/triste.
- No activa el buzzer.
- No mueve los servos.
- Solo muestra el mensaje en pantalla.

Esto evita que el robot haga ruido o movimiento durante la noche.

---

## Comunicación con la app Flutter

La app Flutter se conecta al robot por **BLE**. Esta comunicación se usa principalmente para manejar la agenda.

Ejemplo de comando para crear un recordatorio:

```txt
ADD|1|2026-05-18|14:30|TOMAR MEDICINA
```

Ejemplo de respuesta del robot:

```txt
OK|REMINDER_SAVED
```

---

## Comunicación con el servidor

El servidor se comunica con OttoBot por **WiFi**. Su función principal es enviar comandos remotos y registrar eventos del robot.

El servidor puede usarse para:

- Enviar comandos de movimiento.
- Registrar eventos.
- Guardar logs.
- Monitorear acciones del robot.
- Integrarse con una base de datos como MongoDB.

---

## Comandos de movimiento desde el servidor

Los movimientos especiales del robot son manejados desde el servidor mediante comandos WiFi.

Comandos principales:

```txt
WALK
SHOW_SCAN
SHOW_CUTE
SHOW_WIN
SHOW_KICK
SHOW_BOW
SHOW_STOMP
SHOW_ONE_FOOT
```

Descripción general:
- `WALK`: movimiento hacia adelante del robot
- `SHOW_SCAN`: animación tipo escaneo o búsqueda.
- `SHOW_CUTE`: animación tierna o amigable.
- `SHOW_WIN`: animación de victoria o celebración.
- `SHOW_KICK`: movimiento tipo patada.
- `SHOW_BOW`: reverencia o saludo.
- `SHOW_STOMP`: movimiento tipo pisotón.
- `SHOW_ONE_FOOT`: animación de equilibrio o movimiento sobre un pie.

---

## Comunicación Serial

El robot también puede recibir comandos desde el Monitor Serial de Arduino IDE. Esta opción se usa para pruebas y depuración.

Ejemplos:

```txt
TEST
TEST_ALL
TXT|HOLA SOY OTTOBOT
```

---

## Sensor Touch

El robot incluye un sensor touch capacitivo. Este sensor permite cambiar el estado de ánimo del robot.

Funcionamiento:

- El toque debe mantenerse por 4 segundos.
- Al cumplirse el tiempo, cambia el estado de ánimo.
- Durante un recordatorio activo, el touch se bloquea para no interrumpir la alerta.

Estados de ánimo:

- `DEFAULT`: Los ojos se mantienen abiertos 
- `HAPPY`: Expresión de alegria
- `TIRED`: Emoción triste
- `ANGRY`: Ojos enojados
---

## Qué se muestra en el robot

La pantalla OLED muestra la información principal del robot de forma simple.

Puede mostrar:

- Ojos animados.
- Estado de ánimo.
- Icono de notificación.
- Texto de recordatorios.
- Mensajes enviados desde la app.
- Animaciones visuales.

Ejemplos:

```txt
TOMAR MEDICINA
REUNION A LAS 3
HOLA SOY OTTOBOT
```

Los mensajes largos se muestran como texto desplazable.

---

## Componentes principales

- **Arduino Nano 33 IoT**: microcontrolador principal con BLE y WiFi.
- **Pantalla OLED I2C**: muestra ojos, mensajes y animaciones.
- **RTC DS3231**: mantiene fecha y hora para los recordatorios.
- **Sensor LDR**: detecta luz u oscuridad.
- **Sensor Touch TTP223**: permite cambiar el estado de ánimo.
- **Servomotores SG90**: generan los movimientos físicos del robot.
- **Buzzer**: reproduce sonidos y alertas.
- **App Flutter**: gestiona principalmente la agenda.
- **Servidor WiFi**: envía comandos remotos y registra eventos.
- **MongoDB**: base de datos opcional para logs y registros.

---

## Estructura del proyecto

El código está dividido en varios módulos `.ino`:

```txt
Main.ino          -> Configuración principal y ciclo del robot.
BLEModule.ino     -> Comunicación BLE con la app Flutter.
WiFiEvents.ino    -> Comunicación WiFi con el servidor.
Display.ino       -> Pantalla OLED, ojos y mensajes.
Reminders.ino     -> Gestión de recordatorios.
Sensors.ino       -> LDR y sensor touch.
Servos.ino        -> Movimientos y animaciones.
Buzzer.ino        -> Sonidos y alertas.
Utils.ino         -> Funciones auxiliares.
```

---

## OttoBotAgenda

OttoBot Agenda combina una app móvil, comunicación BLE, servidor WiFi, sensores, pantalla OLED, servos y buzzer para crear una agenda robótica interactiva.

La **app Flutter** se encarga principalmente de manejar los recordatorios. El **servidor** se encarga de enviar comandos especiales de movimiento, como `SHOW_SCAN`, `SHOW_CUTE`, `SHOW_WIN`, `SHOW_KICK`, `SHOW_BOW`, `SHOW_STOMP` y `SHOW_ONE_FOOT`.

El robot muestra recordatorios, reacciona al ambiente con el LDR, cambia de ánimo con el touch y puede ejecutar animaciones físicas para hacerlo más expresivo e interactivo.
