#include <WiFi.h>
#include "DHT.h"

// ============================
// CONFIGURAÇÕES DO DHT11
// ============================
#define DHTPIN 3          // GPIO do DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ============================
// CONFIGURAÇÕES DO SENSOR DE UMIDADE DO SOLO
// ============================
#define SENSOR_UMIDADE_SOLO_PIN 2   // GPIO onde o sensor está conectado
#define SENSOR_MIN 1200             // Valor com o sensor seco
#define SENSOR_MAX 3000             // Valor com o sensor molhado
#define NUM_LEITURAS 10             // Número de leituras para média

// ============================
// CONFIGURAÇÕES DO RELÉ
// ============================
#define RELE_PIN 4  // GPIO conectado ao relé via BC548B
bool estadoRele = LOW;

// ============================
// CONFIGURAÇÕES DE IP ESTÁTICO
// ============================
IPAddress ip(192, 168, 1, 155);       // IP fixo desejado
IPAddress gateway(192, 168, 1, 254);   // IP do roteador (gateway)
IPAddress subnet(255, 255, 255, 0);    // Máscara de sub-rede

// ============================
// CONFIGURAÇÕES DE WI-FI
// ============================
const char* ssids[] = {"pro-5G", "pro-2.4G", "Polícia Federal"};
const char* passwords[] = {"32663765", "32663765", "32663765"};
const int numRedes = sizeof(ssids) / sizeof(ssids[0]);

// ============================
// CONFIGURAÇÕES DO SERVIDOR
// ============================
WiFiServer server(8080);

// ============================
// HTML EMBUTIDO (site completo)
// ============================
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>SmartGarden - Monitoramento</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f0f8f5;
      text-align: center;
      padding: 40px;
    }
    h1 {
      color: #2e7d32;
    }
    .sensor-data, .clima, .chuva, .alerta-chuva, .controle-rele {
      margin-top: 20px;
      font-size: 20px;
      color: #333;
    }
    .barra-umidade {
      width: 80%;
      max-width: 400px;
      height: 25px;
      background-color: #ddd;
      border-radius: 15px;
      margin: 10px auto;
      overflow: hidden;
      box-shadow: inset 0 0 5px rgba(0,0,0,0.2);
    }
    .barra-preenchida {
      height: 100%;
      width: 0%;
      transition: width 0.5s ease-in-out;
      border-radius: 15px;
    }
    .status-solo {
      font-size: 18px;
      margin-top: 5px;
      font-weight: bold;
    }
    .loading {
      color: #666;
      font-style: italic;
      margin-top: 20px;
    }
    .chuva p, .alerta-chuva p {
      margin: 5px 0;
    }
    .alerta-chuva {
      margin-top: 30px;
      padding: 20px;
      border-radius: 10px;
      background-color: #fff3cd;
      color: #856404;
      border: 1px solid #ffeeba;
    }
    .controle-rele button {
      padding: 10px 20px;
      font-size: 16px;
      cursor: pointer;
      background-color: #4caf50;
      color: white;
      border: none;
      border-radius: 5px;
    }
    .controle-rele button:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <h1>🌱 SmartGarden - Monitoramento</h1>

  <!-- Dados do ESP32 -->
  <div class="sensor-data">
    <p><strong>Temperatura:</strong> <span id="temperatura">--</span> °C</p>
    <p><strong>Umidade do Ar (ESP32):</strong> <span id="umidade_ar">--</span> %</p>
    <p><strong>Umidade do Solo:</strong> <span id="umidade_solo">--</span> %</p>
  </div>

  <!-- Barra de umidade do solo -->
  <div class="barra-umidade">
    <div class="barra-preenchida" id="barra-umidade-solo"></div>
  </div>
  <div class="status-solo" id="status_solo">Aguardando dados...</div>

  <!-- Clima atual -->
  <div class="clima">
    <h2>🌤️ Clima na sua cidade</h2>
    <p><strong>Cidade:</strong> <span id="clima-cidade">--</span></p>
    <p><strong>Temperatura:</strong> <span id="clima-temp">--</span> °C</p>
    <p><strong>Umidade do Ar (Clima):</strong> <span id="clima-umidade">--</span> %</p>
    <p><strong>Comparação:</strong> <span id="clima-comparacao">--</span></p>
  </div>

  <!-- Previsão de chuva -->
  <div class="chuva">
    <h2>🌧️ Previsão de Chuva (Próximas 6 horas)</h2>
    <p><strong>Total:</strong> <span id="chuva-6h">--</span> mm</p>
  </div>

  <!-- Alerta de chuva -->
  <div class="alerta-chuva">
    <h2>⚠️ Alerta de Chuva</h2>
    <p id="alerta-mensagem">Carregando dados...</p>
  </div>

  <!-- Controle do Relé -->
  <div class="controle-rele">
    <h2>🔌 Controle do Relé</h2>
    <button id="botaoRele" onclick="toggleRele()">Ligar Relé</button>
    <p id="statusRele">Estado atual: Desligado</p>
  </div>

  <p class="loading" id="status">Carregando dados...</p>

  <script>
    const ipESP32 = "http://192.168.1.155:8080"; // IP do ESP32
    const apiKey = "2b5855631a29821cd5ab48fb1244df10"; // Sua chave da OpenWeatherMap
    const cidade = "Curitiba"; // Nome da cidade
    const latitude = -25.4533; // Latitude (Curitiba)
    const longitude = -49.2011; // Longitude (Curitiba)

    function atualizarDados() {
      // Dados do ESP32
      fetch(`${ipESP32}/api/sensor`)
        .then(response => {
          if (!response.ok) throw new Error("Erro no ESP32");
          return response.json();
        })
        .then(data => {
          document.getElementById("temperatura").textContent = data.temperatura.toFixed(1);
          document.getElementById("umidade_ar").textContent = data.umidade_ar.toFixed(1);
          document.getElementById("umidade_solo").textContent = data.umidade_solo.toFixed(1);

          // Atualiza barra de umidade do solo
          const barra = document.getElementById("barra-umidade-solo");
          const status = document.getElementById("status_solo");
          const umidadeSolo = data.umidade_solo;

          barra.style.width = `${umidadeSolo}%`;

          if (umidadeSolo < 30) {
            barra.style.backgroundColor = "#e53935";
            status.textContent = "⚠️ Solo seco";
            status.style.color = "#c62828";
          } else if (umidadeSolo < 70) {
            barra.style.backgroundColor = "#43a047";
            status.textContent = "✅ Umidade ideal";
            status.style.color = "#2e7d32";
          } else {
            barra.style.backgroundColor = "#1e88e5";
            status.textContent = "💧 Solo muito úmido";
            status.style.color = "#1565c0";
          }

          // Atualiza estado do relé
          fetch(`${ipESP32}/api/rele`)
            .then(response => response.json())
            .then(data => {
              const botao = document.getElementById("botaoRele");
              const status = document.getElementById("statusRele");
              if (data.estado === "ligado") {
                botao.textContent = "Desligar Relé";
                status.textContent = "Estado atual: Ligado";
              } else {
                botao.textContent = "Ligar Relé";
                status.textContent = "Estado atual: Desligado";
              }
            });
        })
        .catch(err => {
          console.error("Falha no ESP32:", err.message);
        });

      // Dados do clima (OpenWeatherMap)
      const urlClima = `https://api.openweathermap.org/data/2.5/weather?lat=${latitude}&lon=${longitude}&appid=${apiKey}&units=metric&lang=pt_br`;

      fetch(urlClima)
        .then(response => {
          if (!response.ok) throw new Error("Erro na API do clima");
          return response.json();
        })
        .then(json => {
          const temp = json.main.temp;
          const umidade = json.main.humidity;
          const nomeCidade = json.name;

          document.getElementById("clima-cidade").textContent = nomeCidade;
          document.getElementById("clima-temp").textContent = temp.toFixed(1);
          document.getElementById("clima-umidade").textContent = umidade.toFixed(1);

          const umidadeESP32 = parseFloat(document.getElementById("umidade_ar").textContent);
          let comparacao = "";

          if (Math.abs(umidadeESP32 - umidade) < 5) {
            comparacao = "✅ Próximo ao clima real";
          } else if (umidadeESP32 < umidade) {
            comparacao = "⚠️ Sensor mais seco que o ambiente";
          } else {
            comparacao = "⚠️ Sensor mais úmido que o ambiente";
          }

          document.getElementById("clima-comparacao").textContent = comparacao;
        });

      // Previsão de chuva (Open-Meteo)
      const urlChuva = ` https://api.open-meteo.com/v1/forecast?latitude=${latitude}&longitude=${longitude}&hourly=precipitation&forecast_days=1`;

      fetch(urlChuva)
        .then(response => response.json())
        .then(json => {
          const chuva = json.hourly.precipitation;
          const agora = new Date().getHours();
          const prox6h = chuva.slice(agora, agora + 6).reduce((a, b) => a + b, 0).toFixed(1);

          document.getElementById("chuva-6h").textContent = prox6h + " mm";

          const valorChuva = parseFloat(prox6h);
          const alerta = document.getElementById("alerta-mensagem");

          if (valorChuva === 0) {
            alerta.textContent = "🌤️ Sem previsão de chuva nas próximas 6 horas.";
            alerta.style.color = "green";
          } else if (valorChuva > 0 && valorChuva <= 2.5) {
            alerta.textContent = "🌦️ Chuva leve prevista. Pode molhar levemente.";
            alerta.style.color = "orange";
          } else if (valorChuva <= 7.5) {
            alerta.textContent = "🌧️ Chuva moderada. Considere levar guarda-chuva.";
            alerta.style.color = "darkorange";
          } else if (valorChuva <= 15) {
            alerta.textContent = "⛈️ Chuva forte! Evite sair se possível.";
            alerta.style.color = "red";
          } else {
            alerta.textContent = "🌊 Chuva muito forte! Risco de alagamentos!";
            alerta.style.color = "darkred";
            alerta.style.fontWeight = "bold";
          }
        })
        .catch(err => {
          console.error("Erro na previsão de chuva:", err);
          document.getElementById("alerta-mensagem").textContent = "⚠️ Erro ao carregar previsão de chuva.";
        });

      document.getElementById("status").textContent = "Dados atualizados.";
    }

    function toggleRele() {
      const botao = document.getElementById("botaoRele");
      const novoEstado = botao.textContent === "Ligar Relé" ? 1 : 0;

      fetch(`${ipESP32}/rele?estado=${novoEstado}`)
        .then(response => response.json())
        .then(data => {
          atualizarDados();
        })
        .catch(err => {
          console.error("Erro ao controlar o relé:", err);
        });
    }

    // Primeira atualização
    atualizarDados();

    // Atualiza a cada 5 segundos
    setInterval(atualizarDados, 5000);
  </script>
</body>
</html>
)rawliteral";

// ============================
// FUNÇÃO PARA LER UMIDADE DO SOLO
// ============================
float lerUmidadeSolo() {
  long soma = 0;
  for (int i = 0; i < NUM_LEITURAS; i++) {
    soma += analogRead(SENSOR_UMIDADE_SOLO_PIN);
    delay(10);
  }
  int valorAnalogico = soma / NUM_LEITURAS;
  float umidade = map(valorAnalogico, SENSOR_MIN, SENSOR_MAX, 100, 0);
  if (umidade < 0) umidade = 0;
  if (umidade > 100) umidade = 100;
  return umidade;
}

// ============================
// CONECTA AO WI-FI COM IP FIXO
// ============================
void conectarWiFi() {
  int tentativa = 0;
  while (WiFi.status() != WL_CONNECTED) {
    const char* ssid = ssids[tentativa % numRedes];
    const char* senha = passwords[tentativa % numRedes];
    Serial.print("Tentando conectar a: ");
    Serial.println(ssid);

    WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, senha);
    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 10) {
      delay(1000);
      Serial.print(".");
      tentativas++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ Conectado com sucesso!");
      Serial.print("IP Atribuído: ");
      Serial.println(WiFi.localIP());
      server.begin();
      break;
    }

    Serial.println("\n❌ Falha. Tentando próxima rede.");
    WiFi.disconnect(true);
    delay(1000);
    tentativa++;
  }
}

// ============================
// SETUP
// ============================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n🚀 Iniciando SmartGarden API...");

  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, estadoRele);

  conectarWiFi();
  dht.begin();
}

// ============================
// LOOP
// ============================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado. Reconectando...");
    conectarWiFi();
  }

  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.readStringUntil('\n'); // pula linha vazia

    // Serve a página principal
    if (request.indexOf("GET / ") >= 0 || request.indexOf("GET /index.html") >= 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.print(htmlPage);
    }

    // API para dados dos sensores
    else if (request.indexOf("GET /api/sensor") >= 0) {
      float temp = dht.readTemperature();
      float umid = dht.readHumidity();
      float umidSolo = lerUmidadeSolo();
      if (isnan(temp)) temp = 0.0;
      if (isnan(umid)) umid = 0.0;

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Access-Control-Allow-Origin: *");
      client.println();
      client.print("{\"temperatura\":");
      client.print(temp);
      client.print(",\"umidade_ar\":");
      client.print(umid);
      client.print(",\"umidade_solo\":");
      client.print(umidSolo);
      client.println("}");
    }

    // API para estado do relé
    else if (request.indexOf("GET /api/rele") >= 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println();
      client.print("{\"estado\":\"");
      client.print(estadoRele == HIGH ? "ligado" : "desligado");
      client.println("\"}");
    }

    // API para controle do relé
    else if (request.indexOf("GET /rele?estado=1") >= 0) {
      estadoRele = HIGH;
      digitalWrite(RELE_PIN, estadoRele);
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println();
      client.println("{\"status\":\"Relé ligado\"}");
    } else if (request.indexOf("GET /rele?estado=0") >= 0) {
      estadoRele = LOW;
      digitalWrite(RELE_PIN, estadoRele);
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println();
      client.println("{\"status\":\"Relé desligado\"}");
    }

    // Página 404
    else {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<h1>404 - Página não encontrada</h1>");
    }

    client.stop();
  }

  delay(10);
}