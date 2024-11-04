#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <centroides.h>

const char* ssid = "NombreRedWIFI";
const char* password = "ContraseñaWIFI";


WebServer server(80);
Adafruit_MPU6050 mpu;

String orientaciones[6] = {"Atrás", "Boca abajo", "Boca arriba", "Adelante", "Lado izquierdo", "Lado derecho"};
String orientacionActual = "Indeterminado";

// Función para detectar la orientación del dispositivo
void detectarOrientacion() {
    sensors_event_t accel;
    mpu.getAccelerometerSensor()->getEvent(&accel);

    float x = accel.acceleration.x;
    float y = accel.acceleration.y;
    float z = accel.acceleration.z;
    
    float minDist = 1000;
    int indiceOrientacion = -1;

    for (int i = 0; i < 6; i++) {
        float dx = x - centroides[i][0];
        float dy = y - centroides[i][1];
        float dz = z - centroides[i][2];
        float distancia = sqrt(dx*dx + dy*dy + dz*dz);
        
        if (distancia < minDist) {
            minDist = distancia;
            indiceOrientacion = i;
        }
    }

    if (indiceOrientacion != -1) {
        orientacionActual = orientaciones[indiceOrientacion];
    }
}

// Página HTML para mostrar la orientación actual con actualización automática
void manejarRaiz() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<title>Orientación del Dispositivo</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; background-color: #f4f4f9; }";
    html += "h1 { color: #333; }";
    html += "p { font-size: 1.2em; color: #555; }";
    html += "strong { font-size: 1.5em; color: #007bff; }";
    html += "</style>";
    html += "<script>";
    html += "function actualizarOrientacion() {";
    html += "  fetch('/orientacion').then(response => response.text()).then(data => {";
    html += "    document.getElementById('orientacion').innerText = data;";
    html += "  });";
    html += "}";
    html += "setInterval(actualizarOrientacion, 1000);";  // Actualizar cada segundo
    html += "</script>";
    html += "</head><body>";
    html += "<div style='text-align: center;'>";
    html += "<h1>Orientación del Dispositivo</h1>";
    html += "<p>Orientación Actual: <strong id='orientacion'>" + orientacionActual + "</strong></p>";
    html += "</div>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}


// Ruta para obtener solo la orientación actual como texto
void manejarOrientacion() {
    server.send(200, "text/plain", orientacionActual);
}

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectado a WiFi. Dirección IP: ");
    Serial.println(WiFi.localIP());

    if (!mpu.begin()) {
        Serial.println("MPU6050 no detectado.");
        while (1);
    }

    server.on("/", HTTP_GET, manejarRaiz);
    server.on("/orientacion", HTTP_GET, manejarOrientacion);  // Nueva ruta para orientación
    server.begin();
    Serial.println("Servidor listo.");
}

void loop() {
    server.handleClient();
    detectarOrientacion();  // Detecta la orientación en cada iteración
    delay(500);  // Intervalo de detección
}