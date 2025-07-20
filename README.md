# 🌱 SmartGarden API – Monitoramento Ambiental com ESP32

Este projeto transforma o ESP32 em uma estação meteorológica inteligente e controladora, com as seguintes funcionalidades:

- Leitura de temperatura e umidade do ar (DHT11)  
- Leitura de umidade do solo (sensor capacitivo)  
- Servidor web embutido com interface HTML + JS  
- Controle remoto de relé (GPIO 4)  
- Previsão do tempo e alertas de chuva (OpenWeatherMap + Open-Meteo)  
- Acesso remoto via Cloudflare Tunnel ou ngrok  

---

## 🧰 Funcionalidades Principais

### ✅ Sensores

- Temperatura e umidade do ar (DHT11)  
- Umidade do solo (sensor analógico)  

### ✅ Controle de Relé

- Aciona um relé via GPIO 4 (com transistor BC548B)  
- Botão na interface web para ligar/desligar  

### ✅ Interface Web

- Servida diretamente pelo ESP32  
- Atualização automática dos dados  
- Barra visual de umidade do solo  
- Comparação entre umidade do sensor e do clima real  
- Alertas visuais de chuva  

### ✅ Conectividade

- Conexão Wi-Fi com múltiplas redes  
- IP estático configurável  
- Acesso remoto via Cloudflare Tunnel ou ngrok  

### ✅ Previsão do Tempo

- Integração com OpenWeatherMap (clima atual)  
- Integração com Open-Meteo (previsão de chuva)  

---

## 📦 Componentes Necessários

| Componente                         | Função                                   |
|-----------------------------------|------------------------------------------|
| ESP32                             | Controlador principal                    |
| DHT11                             | Mede temperatura e umidade do ar         |
| Sensor Capacitivo de Umidade do Solo | Mede a umidade do solo                |
| Relé (12V) + Transistor BC548B     | Aciona dispositivos via ESP32            |
| Fonte de alimentação 12V              | Alimenta ESP32, sensores e rele             |

---

## 🔌 Ligações do Hardware

| Pino ESP32 | Conexão                             |
|------------|-------------------------------------|
| GPIO 3     | DHT11 – Pino de dados               |
| GPIO 2     | Sensor de umidade do solo – Sinal   |
| GPIO 4     | BC548B (base) → Relé                |
| 3.3V       | DHT11 / Sensor de umidade do solo   |
| GND        | Terra comum                         |
| USB/Bateria| Alimentação do ESP32                |

---

## 🌐 Funcionalidades da Interface Web

A interface web embutida no ESP32 exibe:

- Temperatura ambiente  
- Umidade do ar (do sensor e do clima)  
- Umidade do solo (com barra visual)  
- Previsão de chuva (nas próximas 6 horas)  
- Botão para ligar/desligar relé  

---

## 🌍 Acesso Remoto

Acesse a interface web do ESP32 remotamente usando:

- `ngrok http 8080`  
- `cloudflared tunnel --url http://localhost:8080`  

**Endereço local:**  
`http://<IP_DO_ESP32>:8080`

---

## 📋 Requisitos de Software

### 🔧 Para o ESP32:

- [Arduino IDE](https://www.arduino.cc/en/software)  
- Biblioteca **DHT Sensor Library** (Adafruit)  
- Biblioteca **WiFi.h** (inclusa no core do ESP32)  
- **ESP32 Core** instalado no Arduino IDE  

### 🌐 Para o Frontend:

- HTML5  
- CSS3  
- JavaScript (embutido no ESP32)  