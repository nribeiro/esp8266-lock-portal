#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer server(80);

const char* apSSID = "ESP-CMD-PORTAL";

String receivedMessage = "";
const int relay = 5;  //D1

unsigned long lastCommandTime = 0;
const unsigned long commandCooldown = 5000;  // 5 segundos

// Relé não bloqueante
bool isRelayActive = false;
unsigned long relayStartTime = 0;
const unsigned long relayDuration = 200;  // ms

bool ledPulseActive = false;
unsigned long ledPulseStartTime = 0;
const unsigned long ledPulseDuration = 100;  // ms

#define BUTTON_PIN D2
const int buttonPin = 4;  // D2

// Página principal
void handleRoot() {
  Serial.println("handleRoot() called");
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <title>ESP8266 Command Portal</title>
      <style>
        body { font-family: sans-serif; background: #f4f4f4; text-align: center; padding: 2em; }
        button {
          padding: 10px 20px;
          font-size: 1.2em;
          background: #007bff;
          color: white;
          border: none;
          border-radius: 5px;
          cursor: pointer;
        }
        button:disabled {
          background: #ccc;
          cursor: not-allowed;
        }
        #response {
          margin-top: 20px;
          font-weight: bold;
          font-size: 1.1em;
        }
        .success { color: green; }
        .error { color: red; }
        .info { color: #555; }
      </style>
    </head>
    <body>
      <h1>ESP8266 Command Portal</h1>
      <p>Send a command via URL like: <code>/cmd?val=ACTIVATE</code></p>
      <button id="openBtn" onclick="sendCommand()">OPEN</button>
      <div id="response" class="info">Ready</div>

      <script>
        let cooldownInterval = null;

        function sendCommand() {
          const btn = document.getElementById("openBtn");
          const resp = document.getElementById("response");

          btn.disabled = true;
          resp.textContent = "Sending...";
          resp.className = "info";

          fetch('/cmd?val=ACTIVATE')
            .then(res => res.json())
            .then(data => {
              if (data.status === "ok") {
                resp.textContent = data.message;
                resp.className = "success";
                setTimeout(() => {
                  resp.textContent = "Ready";
                  resp.className = "info";
                }, 3000);
              } else if (data.status === "cooldown") {
                let waitTime = parseInt(data.message.match(/(\d+)/)[0]); // extrai segundos
                resp.className = "error";
                startCooldown(waitTime);
              } else {
                resp.textContent = data.message;
                resp.className = "error";
              }
            })
            .catch(err => {
              resp.textContent = "Erro: " + err;
              resp.className = "error";
            })
            .finally(() => {
              if (!cooldownInterval) btn.disabled = false;
            });
        }

        function startCooldown(seconds) {
          const btn = document.getElementById("openBtn");
          const resp = document.getElementById("response");

          let remaining = seconds;
          btn.disabled = true;

          if (cooldownInterval) clearInterval(cooldownInterval);
          cooldownInterval = setInterval(() => {
            if (remaining > 0) {
              resp.textContent = `Cooldown ativo — tenta novamente em ${remaining--}s`;
            } else {
              clearInterval(cooldownInterval);
              cooldownInterval = null;
              resp.textContent = "Ready";
              resp.className = "info";
              btn.disabled = false;
            }
          }, 1000);
        }
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Comando via URL
void handleCommand() {
  Serial.println("handleCommand() called");

  if (server.hasArg("val")) {
    String command = server.arg("val");
    command.trim();
    Serial.println("Command received: " + command);

    unsigned long now = millis();
    if (now - lastCommandTime > commandCooldown) {
      if (command.equals("ACTIVATE")) {
        triggerRelay();
        lastCommandTime = now;

        String json = "{\"status\":\"ok\",\"message\":\"Command executed: ACTIVATE\"}";
        server.send(200, "application/json", json);
      } else {
        String json = "{\"status\":\"error\",\"message\":\"Unknown command: " + command + "\"}";
        server.send(400, "application/json", json);
      }
    } else {
      unsigned long waitTime = (commandCooldown - (now - lastCommandTime)) / 1000;

      String json = "{\"status\":\"cooldown\",\"message\":\"Try again in ";
      json += String(waitTime);
      json += " seconds\"}";
      server.send(429, "application/json", json);
    }
  } else {
    String json = "{\"status\":\"error\",\"message\":\"Missing 'val' parameter in URL\"}";
    server.send(400, "application/json", json);
  }
}

// Redirecionar para /
void handleNotFound() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleButtonPress() {
  static bool buttonWasPressed = false;
  int reading = digitalRead(buttonPin);
  unsigned long now = millis();

  // Se o botão está pressionado (LOW) e ainda não foi processado
  if (reading == LOW && !buttonWasPressed) {
    Serial.println("Botão pressionado!");

    // Verifica o cooldown
    if (now - lastCommandTime > commandCooldown) {
      triggerRelay();
      lastCommandTime = now;
    } else {
      Serial.println("Cooldown ativo — comando ignorado");
    }

    buttonWasPressed = true;  // marca como já processado
  }

  // Quando o botão é libertado, limpa o estado
  if (reading == HIGH && buttonWasPressed) {
    buttonWasPressed = false;
  }
}

// Acionar relé (não bloqueante)
void triggerRelay() {
  digitalWrite(relay, HIGH);
  isRelayActive = true;
  relayStartTime = millis();

  // Piscar o LED
  digitalWrite(LED_BUILTIN, LOW);  // acende (ativo em LOW)
  ledPulseActive = true;
  ledPulseStartTime = millis();
}

// Comunicação via Serial
/*
void handleSerialComm() {
  while (Serial.available()) {
    char incomingChar = Serial.read();

    if (incomingChar == '\n') {
      receivedMessage.trim();
      Serial.print("You sent: ");
      Serial.println(receivedMessage);

      if (receivedMessage.equals("ACTIVATE")) {
        unsigned long now = millis();
        if (now - lastCommandTime > commandCooldown) {
          triggerRelay();
          lastCommandTime = now;
        } else {
          Serial.println("Cooldown ativo — comando ignorado");
        }
      }

      receivedMessage = "";
    } else {
      receivedMessage += incomingChar;
    }
  }
}
*/


// Verificar se já passou o tempo de ativação do relé
void handleRelayTimer() {
  if (isRelayActive && millis() - relayStartTime > relayDuration) {
    digitalWrite(relay, LOW);
    isRelayActive = false;
  }
}

void handleLedPulse() {
  if (ledPulseActive && millis() - ledPulseStartTime > ledPulseDuration) {
    digitalWrite(LED_BUILTIN, HIGH);  // apaga
    ledPulseActive = false;
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(relay, LOW);
  pinMode(relay, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);

  WiFi.softAP(apSSID);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.println(myIP);

  dnsServer.start(DNS_PORT, "*", myIP);

  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Web server started with captive portal");
}

void loop() {
  //handleSerialComm();
  dnsServer.processNextRequest();
  server.handleClient();
  handleRelayTimer();
  handleButtonPress();
  handleLedPulse();
}
