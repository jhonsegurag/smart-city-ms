/**
 * Sistema de Control de Tráfico Inteligente con ESP32
 * 
 * Este sistema utiliza un ESP32 para controlar dos semáforos en una intersección,
 * basándose en datos de sensores en tiempo real y predicciones de tráfico
 * obtenidas de un servicio web externo (smart-city-ms).
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Wire.h>              //Library required for I2C comms (LCD)
#include <LiquidCrystal_I2C.h> //Library for LCD display via I2C
#include <math.h>              //Mathematics library for pow function (CO2 computation)

#define LDR1 12 // LDR Light sensor from traffic light 1 connected in pin A0
#define LDR2 13 // LDR Light sensor from traffic light 2 connected in pin A1
#define CO2 14  // CO2 sensor connected in pin A3
#define P1 1    // Traffic light 1 button connected in pin 1
#define P2 2    // Traffic light 2 button connected in pin 2
#define CNY1 42 // Infrared sensor 1 in traffic light 1 connected in pin 42
#define CNY2 41 // Infrared sensor 2 in traffic light 1 connected in pin 41
#define CNY3 40 // Infrared sensor 3 in traffic light 1 connected in pin 40
#define CNY4 39 // Infrared sensor 4 in traffic light 2 connected in pin 39
#define CNY5 38 // Infrared sensor 5 in traffic light 2 connected in pin 38
#define CNY6 37 // Infrared sensor 6 in traffic light 2 connected in pin 37
#define LR1 5   // Red traffic light 1 connected in pin 5
#define LY1 4   // Yellow traffic light 1 connected in pin 4
#define LG1 6   // Green traffic light 1 connected in pin 6
#define LR2 7   // Red traffic light 2 connected in pin 7
#define LY2 15  // Yellow traffic light 2 connected in pin 15
#define LG2 16  // Green traffic light 2 connected in pin 16

// Configuración de la conexión WiFi
const char* ssid = "WIFI_SSID"; // Reemplazar con el nombre de la red WiFi
const char* password = "**********";  // Reemplazar con la contrasena de la red WiFi

// URL del servicio de predicciones de tráfico
const char* smartCityServiceUrl = "http://smart-city-ms/api";
const char* PredictionsPath = "/predictions";
const char* TrafficDataPath = "/traffic";

// Constant definitions
//->CO2
const float DC_GAIN = 8.5;                                                               // define the DC gain of amplifier CO2 sensor
const float ZERO_POINT_VOLTAGE = 0.265;                                                  // define the output of the sensor in volts when the concentration of CO2 is 400PPM
const float REACTION_VOLTAGE = 0.059;                                                    // define the “voltage drop” of the sensor when move the sensor from air into 1000ppm CO2
const float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))}; // Line curve with 2 points

// Variable definitions
float volts = 0; // Variable to store current voltage from CO2 sensor
float co2 = 0;   // Variable to store CO2 value

// Library definitions
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Configuración de pines para semáforos
// Semáforo 1
const int LR1 = 0;  // Luz Roja 1
const int LY1 = 0;  // Luz Amarilla 1
const int LG1 = 0;  // Luz Verde 1

// Semáforo 2
const int LR2 = 0;  // Luz Roja 2
const int LY2 = 0; // Luz Amarilla 2
const int LG2 = 0; // Luz Verde 2

// Sensores infrarrojos de tráfico
const int vCNY1 = false; // Sensor infrarrojo 1
const int vCNY2 = false; // Sensor infrarrojo 2
const int vCNY3 = false; // Sensor infrarrojo 3
const int vCNY4 = false; // Sensor infrarrojo 4
const int vCNY5 = false; // Sensor infrarrojo 5
const int vCNY6 = false; // Sensor infrarrojo 6

// Pulsadores (peatones)
const int vP1 = false;  // Pulsador peatonal 1
const int vP2 = false;  // Pulsador peatonal 2

// Sensores de luz ambiente
const int vLDR1 = 0; // Sensor de luz 1
const int vLDR2 = 0; // Sensor de luz 2

// Sensor de CO2
const int vCO2 = 0;  // Sensor de monóxido de carbono

// Variables para almacenar estados
int trafficIntensity1 = 0; // Intensidad de tráfico en dirección 1
int trafficIntensity2 = 0; // Intensidad de tráfico en dirección 2
int lightLevel1 = 0;       // Nivel de luz ambiental 1
int lightLevel2 = 0;       // Nivel de luz ambiental 2
int co2Level = 0;          // Nivel de CO2
bool pedestrianRequest1 = false; // Solicitud de peatones 1
bool pedestrianRequest2 = false; // Solicitud de peatones 2

// Tiempos base para las fases de los semáforos (en milisegundos)
unsigned long baseGreenTime1 = 20000;  // Tiempo base verde para semáforo 1
unsigned long baseGreenTime2 = 20000;  // Tiempo base verde para semáforo 2
unsigned long yellowTime = 3000;       // Tiempo amarillo para ambos semáforos

// Tiempos dinámicos actuales
unsigned long currentGreenTime1;
unsigned long currentGreenTime2;

// Variables para control de tiempo
unsigned long previousMillis = 0;
unsigned long interval = 10000; // Intervalo para consultar el servicio web (10 segundos)

// Estados del ciclo de semáforo
enum TrafficLightState {
  GREEN_1,
  YELLOW_1,
  RED_1_GREEN_2,
  RED_1_YELLOW_2
};

TrafficLightState currentState = GREEN_1;
unsigned long stateStartTime = 0;

// Estructura para almacenar predicciones
struct TrafficPrediction {
  int predictedTraffic1;
  int predictedTraffic2;
  int suggestedTiming1;
  int suggestedTiming2;
};

TrafficPrediction currentPrediction = {0, 0, 20, 20}; // Valores iniciales por defecto

void setup() {
  
  // Inicialización de pines para semáforos
  pinMode(LR1, OUTPUT);
  pinMode(LY1, OUTPUT);
  pinMode(LG1, OUTPUT);
  pinMode(LR2, OUTPUT);
  pinMode(LY2, OUTPUT);
  pinMode(LG2, OUTPUT);
  
  // Inicialización de sensores infrarrojos como entradas
  pinMode(CNY1, INPUT);
  pinMode(CNY2, INPUT);
  pinMode(CNY3, INPUT);
  pinMode(CNY4, INPUT);
  pinMode(CNY5, INPUT);
  pinMode(CNY6, INPUT);
  
  // Inicialización de pulsadores con resistencia pull-up interna
  pinMode(P1, INPUT);
  pinMode(P2, INPUT);
  
  // Inicialización de sensores de luz y CO2 como entradas analógicas
  pinMode(LDR1, INPUT);
  pinMode(LDR2, INPUT);
  pinMode(CO2, INPUT);

  // Inicializacion en apagado por prevencion
  digitalWrite(LR1, LOW); // Turn Off Red traffic light 1
  digitalWrite(LY1, LOW); // Turn Off Yellow traffic light 1
  digitalWrite(LG1, LOW); // Turn Off Green traffic light 1
  digitalWrite(LR2, LOW); // Turn Off Red traffic light 2
  digitalWrite(LY2, LOW); // Turn Off Yellow traffic light 2
  digitalWrite(LG2, LOW); // Turn Off Green traffic light 2
  
  // Conexión a WiFi
  connectToWifi();
  
  // Estado inicial: semáforo 1 en verde, semáforo 2 en rojo
  setTrafficLight1(HIGH, LOW, LOW); // Rojo, Amarillo, Verde
  setTrafficLight2(LOW, LOW, HIGH);
  stateStartTime = millis();
  
  // Obtener la primera predicción
  getPredictionSmartCity();

  Serial.begin(9600);
  lcd.init();
  lcd.backlight(); // Turn on LCD backlight
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Leer sensores
  readSensors();
  updateLCD();
  
  // Comprobar si hay solicitudes de peatones
  checkPedestrianRequests();
  
  // Consultar servicio de predicciones cada cierto tiempo
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    getPredictionSmartCity();
    adjustTimings();
  }
  
  // Control del ciclo de semáforos
  controlTrafficLights(currentMillis);
  
  // Ajustar brillo de las luces según nivel de luz ambiental
  //adjustLightBrightness();
  
  // Enviar datos a la nube para análisis
  if (currentMillis - previousMillis >= interval) {
    sendDataSmartCity();
  }
  
  // Pequeña pausa para estabilidad
  delay(100);
}

void connectToWifi() {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println("Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Fallo en la conexión WiFi. Operando en modo local.");
  }
}

void readSensors() {
    // Leer sensores infrarrojos y calcular intensidad de tráfico
    int vCNY1 = !digitalRead(CNY1); // Invertido porque suelen activarse en LOW
    int vCNY2 = !digitalRead(CNY2);
    int vCNY3 = !digitalRead(CNY3);
    int vCNY4 = !digitalRead(CNY4);
    int vCNY5 = !digitalRead(CNY5);
    int vCNY6 = !digitalRead(CNY6);
    
    // Calcular intensidad de tráfico en cada dirección (0-6)
    trafficIntensity1 = vCNY1 + vCNY2 + vCNY3;
    trafficIntensity2 = vCNY4 + vCNY5 + vCNY6;
    
    // Leer sensores de luz ambiente
    vLDR1 = analogRead(LDR1);
    vLDR2 = analogRead(LDR2);
    lightLevel1 = vLDR1;
    lightLevel2 = vLDR2;
    
    // Leer sensor de CO2
    vCO2 = analogRead(CO2);
    co2Level = vCO2;
  
    // Imprimir resultados para depuración
    Serial.print("Tráfico 1: ");
    Serial.print(trafficIntensity1);
    Serial.print(" | Tráfico 2: ");
    Serial.print(trafficIntensity2);
    Serial.print(" | CO2: ");
    Serial.println(vCO2);
}

void checkPedestrianRequests() {
  // Los pulsadores utilizan resistencias pull-up, se activan cuando se detecta un LOW
  if (digitalRead(vP1) == LOW) {
    pedestrianRequest1 = true;
    Serial.println("Solicitud de peatón en cruce 1");
  }
  
  if (digitalRead(vP2) == LOW) {
    pedestrianRequest2 = true;
    Serial.println("Solicitud de peatón en cruce 2");
  }
}

void getPredictionSmartCity() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Construir URL con parámetros de tráfico actual
    String url = String(smartCityServiceUrl);
    url += "?traffic1=" + String(trafficIntensity1);
    url += "&traffic2=" + String(trafficIntensity2);
    url += "&co2=" + String(vCO2);
    
    http.begin(url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Respuesta del servicio: " + payload);
      
      // Parsear respuesta JSON
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        currentPrediction.predictedTraffic1 = doc["predictedTraffic1"];
        currentPrediction.predictedTraffic2 = doc["predictedTraffic2"];
        currentPrediction.suggestedTiming1 = doc["suggestedTiming1"];
        currentPrediction.suggestedTiming2 = doc["suggestedTiming2"];
        
        Serial.println("Predicción actualizada");
      } else {
        Serial.print("Error al parsear JSON: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("Error en la petición HTTP: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi desconectado, usando modo local");
  }
}

void updateLCD(){
  if (millis() - lastime > 1000)
  {
    lastime = millis();
    // lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LDR1       CNY 1 2 3");
    lcd.setCursor(0, 1);
    lcd.print("LDR2               ");
    lcd.setCursor(0, 2);
    lcd.print("CO2        CNY 4 5 6");
    lcd.setCursor(0, 3);
    lcd.print("P1   P2            ");
    lcd.setCursor(5, 0);
    lcd.print(vLDR1);
    lcd.setCursor(5, 1);
    lcd.print(vLDR2);
    lcd.setCursor(5, 2);
    lcd.print(vCO2);
    lcd.setCursor(3, 3);
    lcd.print(1 * vP1);
    lcd.setCursor(8, 3);
    lcd.print(1 * vP2);
    lcd.setCursor(15, 1);
    lcd.print(1 * vCNY1);
    lcd.setCursor(17, 1);
    lcd.print(1 * vCNY2);
    lcd.setCursor(19, 1);
    lcd.print(1 * vCNY3);
    lcd.setCursor(15, 3);
    lcd.print(1 * vCNY4);
    lcd.setCursor(17, 3);
    lcd.print(1 * vCNY5);
    lcd.setCursor(19, 3);
    lcd.print(1 * vCNY6);
  }
}

void adjustTimings() {
  // Ajustar tiempos basados en predicciones y lecturas actuales
  
  // Factor de tráfico actual (0-6 sensores)
  float trafficFactor1 = map(trafficIntensity1, 0, 3, 1, 2);
  float trafficFactor2 = map(trafficIntensity2, 0, 3, 1, 2);
  
  // Factor de predicción (normalizado)
  float predictionFactor1 = map(currentPrediction.predictedTraffic1, 0, 100, 1, 1.5);
  float predictionFactor2 = map(currentPrediction.predictedTraffic2, 0, 100, 1, 1.5);
  
  // Combinar factores (actual y predicción)
  float combinedFactor1 = (trafficFactor1 * 0.7) + (predictionFactor1 * 0.3);
  float combinedFactor2 = (trafficFactor2 * 0.7) + (predictionFactor2 * 0.3);
  
  // Ajustar tiempos (más tráfico = más tiempo en verde)
  currentGreenTime1 = baseGreenTime1 * combinedFactor1;
  currentGreenTime2 = baseGreenTime2 * combinedFactor2;
  
  // Limitar a rangos razonables
  currentGreenTime1 = constrain(currentGreenTime1, 5000, 45000);
  currentGreenTime2 = constrain(currentGreenTime2, 5000, 45000);
  
  // Considerar solicitudes de peatones
  if (pedestrianRequest1 && currentState != GREEN_1) {
    // Reducir tiempo de espera si hay peatones esperando
    currentGreenTime2 = min(currentGreenTime2, 15000);
  }
  
  if (pedestrianRequest2 && (currentState == GREEN_1 || currentState == YELLOW_1)) {
    // Reducir tiempo de espera si hay peatones esperando
    currentGreenTime1 = min(currentGreenTime1, 15000);
  }
  
  // Considerar niveles de CO2
  // Si el nivel de CO2 es alto, reducir los tiempos de los semáforos para mejorar el flujo
  if (vCO2 > 2500) { // Umbral ajustable
    currentGreenTime1 = currentGreenTime1 * 0.8;
    currentGreenTime2 = currentGreenTime2 * 0.8;
  }
  
  Serial.print("Tiempo verde 1 ajustado: ");
  Serial.print(currentGreenTime1);
  Serial.print("ms | Tiempo verde 2 ajustado: ");
  Serial.print(currentGreenTime2);
  Serial.println("ms");
}

void controlTrafficLights(unsigned long currentMillis) {
  unsigned long elapsedTime = currentMillis - stateStartTime;
  
  switch (currentState) {
    case GREEN_1:
      // Semáforo 1 en verde, semáforo 2 en rojo
      setTrafficLight1(LOW, LOW, HIGH);
      setTrafficLight2(HIGH, LOW, LOW);
      
      // Verificar si es tiempo de cambiar a amarillo
      if (elapsedTime >= currentGreenTime1) {
        currentState = YELLOW_1;
        stateStartTime = currentMillis;
        Serial.println("Cambio a AMARILLO_1");
      }
      break;
      
    case YELLOW_1:
      // Semáforo 1 en amarillo, semáforo 2 sigue en rojo
      setTrafficLight1(LOW, HIGH, LOW);
      setTrafficLight2(HIGH, LOW, LOW);
      
      // Cambiar a semáforo 2 en verde después del tiempo amarillo
      if (elapsedTime >= yellowTime) {
        currentState = RED_1_GREEN_2;
        stateStartTime = currentMillis;
        Serial.println("Cambio a ROJO_1_VERDE_2");
        // Resetear la solicitud de peatones 2 ya que ahora pueden cruzar
        pedestrianRequest2 = false;
      }
      break;
      
    case RED_1_GREEN_2:
      // Semáforo 1 en rojo, semáforo 2 en verde
      setTrafficLight1(HIGH, LOW, LOW);
      setTrafficLight2(LOW, LOW, HIGH);
      
      // Verificar si es tiempo de cambiar a amarillo
      if (elapsedTime >= currentGreenTime2) {
        currentState = RED_1_YELLOW_2;
        stateStartTime = currentMillis;
        Serial.println("Cambio a ROJO_1_AMARILLO_2");
      }
      break;
      
    case RED_1_YELLOW_2:
      // Semáforo 1 sigue en rojo, semáforo 2 en amarillo
      setTrafficLight1(HIGH, LOW, LOW);
      setTrafficLight2(LOW, HIGH, LOW);
      
      // Volver al inicio del ciclo
      if (elapsedTime >= yellowTime) {
        currentState = GREEN_1;
        stateStartTime = currentMillis;
        Serial.println("Cambio a VERDE_1");
        // Resetear la solicitud de peatones 1 ya que ahora pueden cruzar
        pedestrianRequest1 = false;
      }
      break;
  }
}

void setTrafficLight1(int red, int yellow, int green) {
  digitalWrite(LR1, red);
  digitalWrite(LY1, yellow);
  digitalWrite(LG1, green);
}

void setTrafficLight2(int red, int yellow, int green) {
  digitalWrite(LR2, red);
  digitalWrite(LY2, yellow);
  digitalWrite(LG2, green);
}

// void adjustLightBrightness() {
//   // Este método ajustaría el brillo de los LEDs de los semáforos según la luz ambiental
//   // Implementación básica usando mapeo de valores
//   int brightness1 = map(lightLevel1, 0, 4095, 255, 50); // Más brillo cuando está oscuro
//   int brightness2 = map(lightLevel2, 0, 4095, 255, 50);
  
//   // En una implementación real, esto utilizaría PWM para ajustar el brillo
//   // Para simplificar, solo mostramos los valores calculados
//   Serial.print("Brillo semáforo 1: ");
//   Serial.print(brightness1);
//   Serial.print(" | Brillo semáforo 2: ");
//   Serial.println(brightness2);
  
//   // Para implementar PWM real:
//   // ledcWrite(pwmChannel1, brightness1);
//   // ledcWrite(pwmChannel2, brightness2);
// }

void sendDataSmartCity() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // URL del servicio para enviar datos
    const char* dataServiceUrl = smartCityServiceUrl + String(TrafficDataPath);
    
    // Crear JSON con los datos actuales
    DynamicJsonDocument doc(1024);
    doc["timestamp"] = millis();
    doc["trafficIntensity1"] = trafficIntensity1;
    doc["trafficIntensity2"] = trafficIntensity2;
    doc["co2Level"] = vCO2;
    doc["lightLevel1"] = lightLevel1;
    doc["lightLevel2"] = lightLevel2;
    doc["currentGreenTime1"] = currentGreenTime1;
    doc["currentGreenTime2"] = currentGreenTime2;
    doc["pedestrianRequest1"] = pedestrianRequest1;
    doc["pedestrianRequest2"] = pedestrianRequest2;
    
    String jsonData;
    serializeJson(doc, jsonData);
    
    http.begin(dataServiceUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
      Serial.print("Datos enviados exitosamente, código: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error al enviar datos: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  }
}
