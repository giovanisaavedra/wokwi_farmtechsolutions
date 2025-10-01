# 🌱 FarmTech Solutions - Sistema de Irrigação Inteligente

![FarmTech Banner](https://img.shields.io/badge/FarmTech-Solutions-green?style=for-the-badge&logo=leaf)  
![ESP32](https://img.shields.io/badge/ESP32-DevKit_V1-blue?style=flat-square&logo=espressif)  
![Arduino](https://img.shields.io/badge/Arduino-Framework-orange?style=flat-square&logo=arduino)  
![PlatformIO](https://img.shields.io/badge/PlatformIO-6.1.18-purple?style=flat-square)  
![Python](https://img.shields.io/badge/Python-3.11+-yellow?style=flat-square&logo=python)  
![R](https://img.shields.io/badge/R-4.5+-blue?style=flat-square&logo=r)  
![License](https://img.shields.io/badge/License-MIT-red?style=flat-square)  

---

## 🎯 Sobre o Projeto
Sistema de irrigação automatizada e inteligente desenvolvido pela **FarmTech Solutions** como parte do projeto de **Agricultura Digital** (FIAP 2025).  
O sistema monitora **nutrientes (NPK), pH do solo (via LDR), umidade (via DHT22 ou simulação)** e controla automaticamente uma bomba de irrigação.  
Além disso, integra **dados climáticos (OpenWeather + Python + Supabase)** e aplica **análise estatística em R (regressão linear)** para prever quando o solo ficará seco.  

---

## 🍅 Escolha da Cultura: Tomate
O tomate foi escolhido como cultura-alvo por ser altamente sensível a **umidade**, **pH** e **nutrição NPK**.  

📊 Parâmetros ideais:  
- 🧪 **pH:** 6.0 – 6.8  
- 💧 **Umidade do solo:** 60% – 80%  
- 🌱 **NPK:** completo em todas as fases  
- 🌡️ **Temperatura:** 18°C – 26°C  

---

## 🛠️ Componentes Utilizados

### 🔧 Hardware (simulado no Wokwi)
- **ESP32 DevKit V1** (microcontrolador principal)  
- **3 botões verdes** → simulam sensores N, P e K  
- **LDR + resistor 10k** → simula pH do solo  
- **DHT22** → simula umidade/temperatura  
- **Módulo Relé** → representa bomba d’água  

### 💻 Software
- **C++ (Arduino / PlatformIO)** → lógica embarcada no ESP32  
- **Python** → coleta dados meteorológicos (OpenWeather) e envia para Supabase  
- **R** → regressão linear para prever tendência da umidade  

---

## 🏗️ Arquitetura
ESP32 (C++) ──▶ Supabase ◀── Python (API Weather)
│
▼
R (regressão linear)


---

## 🧠 Lógica de Funcionamento

- **No Wokwi (simulação):**
  - Umidade fictícia varia entre 30–90%.  
  - Se **umidade < 60%** → bomba LIGADA 🚰.  
  - Caso contrário → bomba DESLIGADA.  
  - NPK + pH estão presentes apenas para **exibição didática** no Serial Monitor.  

- **Na versão real (ESP32 físico):**
  - Leitura direta de DHT22 + LDR + botões.  
  - Dados enviados ao Supabase.  
  - Integração Python verifica previsão de chuva.  
  - Análise em R prevê tendência da umidade.  

---

## 🔌 Circuito e Conexões

### Conexões principais
- **Botões N, P, K** → GPIO 25, 26, 27  
- **LDR (divisor com 10k)** → GPIO 34  
- **DHT22 (data)** → GPIO 23  
- **Relé (bomba)** → GPIO 21  

📸 Screenshot do circuito Wokwi: *(PRINT VS CODE)*  

<img width="1254" height="582" alt="image" src="https://github.com/user-attachments/assets/99f90a15-0667-4999-b0b9-b2367de45250" />


## 🚀 Como Executar

### ESP32 (Wokwi - simulação)
1. Abra `prog1_local.ino`  
2. Rode no **Wokwi** com `diagram.json` incluso  
3. Veja no **Serial Monitor**:

N: OK | P: Baixo | K: OK
pH do solo: 6.2
Umidade: 45.0 %
Bomba: LIGADA 🚰


### ESP32 (Real)
- Use `prog1_cloud.ino` (conecta Wi-Fi, envia para Supabase).  
- Necessário editar **SSID/senha Wi-Fi** e **API key Supabase**.  

🚀 Como Executar
🔹 Python (API Meteorológica)
cd python
pip install -r requirements.txt
python api_weather.py

Coleta dados da API OpenWeather

Envia previsões para o Supabase

Permite integrar as condições climáticas no sistema de irrigação

🔹 R (Análise Estatística)

source("analise.R")

Gera gráfico com:

🔵 Pontos azuis = leituras reais de umidade (ESP32 → Supabase)

🔴 Linha vermelha = tendência prevista pela regressão linear

➖ Linhas horizontais = faixa ótima (60%–80%) para tomate 🍅

📂 Estrutura do Projeto
.
├── README.md
├── platformio.ini
├── diagram.json
├── /esp32
│   ├── prog1_local.ino   # versão Wokwi (simulação)
│   └── prog1_cloud.ino   # versão real (Wi-Fi + Supabase)
├── /python
│   ├── api_weather.py
│   └── requirements.txt
└── /r
    └── analise.R


🎥 Demonstração

Vídeo (até 5 minutos) mostrando:

Circuito no Wokwi

Serial Monitor com sensores variando

Python coletando previsão meteorológica

Dados chegando no Supabase

Análise em R com regressão linear

👥 Equipe

Márcio Elifas

Leno Siqueira

Yasmim

Giovani

📚 Referências

Embrapa Hortaliças (2023) – Sistemas de Irrigação para Tomate

FAO (2024) – Water Management in Vegetable Production

OpenWeatherMap API – openweathermap.org/api

ESP32 Documentation – docs.espressif.com

Wokwi Simulator – wokwi.com
