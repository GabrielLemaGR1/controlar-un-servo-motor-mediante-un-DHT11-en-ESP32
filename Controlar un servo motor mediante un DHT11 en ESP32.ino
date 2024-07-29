// Incluimos las librerías
#include <WiFi.h>
#include <WebServer.h>
#include "DHTesp.h"
#include <ESP32Servo.h>

// Declaramos el variable que almacena el pin a conectar el DHT11
int pinDHT = 15;

// Declaramos el pin del servomotor
int servoPin = 14;
// Instanciamos el DHT y el servomotor
DHTesp dht;
Servo servo;

// Credenciales de la red WiFi
const char* ssid = "11T Pro";  // Reemplaza con el nombre de tu red WiFi
const char* password = "Lema2023";  // Reemplaza con la contraseña de tu red WiFi

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Inicializamos el dht
  dht.setup(pinDHT, DHTesp::DHT11);

  // Inicializamos el servomotor
  servo.attach(servoPin);

  // Conectar a la red WiFi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("Nuevo cliente.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Obtenemos el arreglo de datos (humedad y temperatura)
            TempAndHumidity data = dht.getTempAndHumidity();
            
            if (isnan(data.humidity) || isnan(data.temperature)) {
              Serial.println("Error al leer del DHT11!");
              return;
            }
            
            // Mover el servomotor en función de la temperatura
            int angle = map(data.temperature, 0, 40, 0, 180);
            servo.write(angle);

            // Enviar una respuesta HTTP al cliente
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            
            client.print("<!DOCTYPE html><html>");
            client.print("<head><meta name='viewport' content='width=device-width, initial-scale=1'>");
            client.print("<link rel='icon' href='data:,'>");
            client.print("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.print(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.print("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.print(".button2 {background-color: #555555;}</style></head>");
            
            // HTML Body
            client.print("<body><h1>Temperatura y Posición del Servomotor</h1>");
            client.print("<p>Temperatura: ");
            client.print(String(data.temperature, 2));
            client.print(" °C</p>");
            client.print("<p>Humedad: ");
            client.print(String(data.humidity, 1));
            client.print(" %</p>");
            client.print("<p>Posición del Servomotor: ");
            client.print(angle);
            client.print(" grados</p>");
            
            client.print("</body></html>");
            client.println();
            
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Cliente desconectado.");
  }
}