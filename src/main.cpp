/*
  ESP32 + DS18B20 + Sensor de umidade do solo
  Exibe temperatura e umidade do solo no Serial Monitor (terminal da IDE)

  Ligações:
    DS18B20:
      VDD  -> 3.3V
      GND  -> GND
      DATA -> GPIO4  (com resistor pull-up de 4.7kΩ entre DATA e 3.3V)

    Sensor de umidade do solo:
      VCC  -> 3.3V
      GND  -> GND
      AOUT -> GPIO34

  Bibliotecas necessárias:
    - OneWire        (por Jim Studt / Paul Stoffregen)
    - DallasTemperature (por Miles Burton)

  Observação: este é um arquivo .cpp "puro" (fora da Arduino IDE), por isso
  o include de Arduino.h abaixo é necessário para setup(), loop(), Serial etc.
  Se estiver usando PlatformIO, salve este arquivo como src/main.cpp.
*/
#include <WiFi.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WebServer.h>

// ---------- Configuração dos pinos ----------
#define ONE_WIRE_BUS 5 // pino de dados do DS18B20
#define SOIL_PIN 34    // pino analógico do sensor de umidade do solo

// ---------- Calibração do sensor de umidade do solo ----------
// Esses valores variam de sensor para sensor. Para calibrar:
//   1) Deixe o sensor seco, ao ar livre, e anote o valor bruto lido (SOIL_DRY)
//   2) Mergulhe a ponta do sensor em água (ou solo bem molhado) e anote (SOIL_WET)
#define SOIL_DRY 3000
#define SOIL_WET 1200

// ---------- Objetos dos sensores ----------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char *ssid = "Ap_2101";
const char *password = "Andre92074072";

WebServer server(80);
void paginainicial()
{
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Monitoramento do Solo</title>
    </head>
    <body>

        <h1>Monitoramento do Solo</h1>

        <p>Temperatura: -- °C</p>
        <p>Umidade: -- %</p>

    </body>
    </html>
    )rawliteral";

  server.send(200, "text/html", html);
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", paginainicial);
  server.begin();
  {
    /* code */
  }

  sensors.begin();

  Serial.println();
  Serial.println("=== Monitor de temperatura e umidade do solo ===");

  int qtdSensores = sensors.getDeviceCount();
  Serial.print("Sensores DS18B20 encontrados: ");
  Serial.println(qtdSensores);

  if (qtdSensores == 0)
  {
    Serial.println("Aviso: nenhum DS18B20 detectado. Verifique a ligação e o resistor pull-up de 4.7kΩ.");
  }
}

void loop()
{
  server.handleClient();
  // ---------- Leitura de temperatura ----------
  sensors.requestTemperatures();
  float temperaturaC = sensors.getTempCByIndex(0);

  // ---------- Leitura de umidade do solo ----------
  int leituraBruta = analogRead(SOIL_PIN);
  int umidadePercentual = map(leituraBruta, SOIL_DRY, SOIL_WET, 0, 100);
  umidadePercentual = constrain(umidadePercentual, 0, 100);

  // ---------- Exibição no terminal ----------
  Serial.println("-----------------------------------------");

  if (temperaturaC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Temperatura: erro na leitura (sensor desconectado)");
  }
  else
  {
    Serial.print("Temperatura: ");
    Serial.print(temperaturaC, 2);
    Serial.println(" C");
  }

  Serial.print("Umidade do solo: ");
  Serial.print(umidadePercentual);
  Serial.print(" %  (leitura bruta: ");
  Serial.print(leituraBruta);
  Serial.println(")");

  delay(2000);
}
