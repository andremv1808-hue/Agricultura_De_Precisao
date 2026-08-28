
#include <WiFi.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WebServer.h>

#define ONE_WIRE_BUS 5 // pino de dados do DS18B20
#define SOIL_PIN 34    // pino analógico do sensor de umidade do solo

#define SOIL_DRY 3000
#define SOIL_WET 1200

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
const char *ssid = "";
const char *password = "";
WebServer server(80);

float temperaturaC = 0.0;
int umidadePercentual = 0;
int leituraBruta = 0;

unsigned long ultimoTempo = 0;
const unsigned long intervalo = 2000;

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

        <p id = "temperatura"></p>°C
        <p id = "umidade"></p>%

        <script>
        
        function atualizarDados(){
        fetch('dados')

          .then(response => response.json())

          .then(data => {
            
            document.getElementById('temperatura').textContent = data.temperatura.toFixed(2);
            
            document.getElementById('umidade').textContent = data.umidade;
          })
            .catch(error => {
            console.error('Erro ao buscar dados:', error);
          });
        }

        atualizarDados();
        
        setInterval(atualizarDados, 1);

        </script>

    </body>
    </html>
    )rawliteral";

  server.send(200, "text/html", html);
}

void enviarDados()
{
  String json = "{";
  json += "\"temperatura\":";
  json += String(temperaturaC, 2);
  json += ",";

  json += "\"umidade\":";
  json += String(umidadePercentual);
  json += "}";

  server.send(200, "applicating/json", json);
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

  // server routes
  server.on("/", paginainicial);
  server.on("/dados", enviarDados);

  // iniciar servidor
  server.begin();

  // inicia os sensores
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

  if (millis() - ultimoTempo >= intervalo)
  {
    ultimoTempo = millis();

    // Temperatura
    sensors.requestTemperatures();
    temperaturaC = sensors.getTempCByIndex(0);

    // Umidade
    leituraBruta = analogRead(SOIL_PIN);

    umidadePercentual = map(leituraBruta, SOIL_DRY, SOIL_WET, 0, 100);
    umidadePercentual = constrain(umidadePercentual, 0, 100);
  }

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
