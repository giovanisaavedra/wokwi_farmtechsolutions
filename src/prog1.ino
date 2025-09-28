/*
 * FarmTech Solutions - Sistema de Irrigação Inteligente
 * Projeto: Monitoramento NPK, pH e Umidade para controle automatizado de irrigação
 * Cultura escolhida: TOMATE (pH ideal: 6.0-6.8, Umidade: 60-80%)
 * 
 * Sensores:
 * - 3 Botões: Simulam sensores NPK
 * - LDR: Simula sensor de pH (0-14)
 * - DHT22: Simula sensor de umidade do solo
 * - Relay: Controla bomba d'água
 * - LED: Indica status do sistema
 */

#include "DHT.h"

// ===== DEFINIÇÕES DE PINOS =====
#define BOTAO_N    2    // Botão Nitrogênio
#define BOTAO_P    4    // Botão Fósforo  
#define BOTAO_K    5    // Botão Potássio
#define DHT_PIN    15   // Sensor DHT22
#define RELAY_PIN  18   // Módulo Relé (Bomba)
#define LED_PIN    19   // LED de Status
#define LDR_PIN    A0   // Sensor LDR (pH)

// ===== CONFIGURAÇÕES =====
#define DHT_TYPE   DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// ===== PARÂMETROS DA CULTURA (TOMATE) =====
const float PH_MIN = 6.0;      // pH mínimo ideal
const float PH_MAX = 6.8;      // pH máximo ideal
const float UMIDADE_MIN = 60.0; // Umidade mínima (%)
const float UMIDADE_MAX = 80.0; // Umidade máxima (%)

// ===== VARIÁVEIS GLOBAIS =====
bool nitrogenio_ok = false;
bool fosforo_ok = false;
bool potassio_ok = false;
float ph_atual = 0.0;
float umidade_atual = 0.0;
bool bomba_ligada = false;
unsigned long ultimo_check = 0;
const unsigned long INTERVALO_CHECK = 2000; // Verifica a cada 2 segundos

// ===== FUNÇÃO DE SETUP =====
void setup() {
  Serial.begin(115200);
  Serial.println("=== FarmTech Solutions - Sistema de Irrigação ===");
  Serial.println("Cultura: TOMATE");
  Serial.println("pH ideal: 6.0-6.8 | Umidade ideal: 60-80%");
  Serial.println("=====================================");
  
  // Configuração dos pinos
  pinMode(BOTAO_N, INPUT_PULLUP);
  pinMode(BOTAO_P, INPUT_PULLUP);  
  pinMode(BOTAO_K, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Inicialização
  digitalWrite(RELAY_PIN, LOW);  // Bomba desligada
  digitalWrite(LED_PIN, LOW);    // LED desligado
  dht.begin();
  
  delay(2000); // Aguarda estabilização dos sensores
  Serial.println("Sistema inicializado com sucesso!");
}

// ===== FUNÇÃO PRINCIPAL =====
void loop() {
  if (millis() - ultimo_check >= INTERVALO_CHECK) {
    lerSensores();
    processarLogicaIrrigacao();
    exibirStatus();
    ultimo_check = millis();
  }
  
  // Pisca LED para indicar sistema ativo
  piscaLedStatus();
}

// ===== LEITURA DOS SENSORES =====
void lerSensores() {
  // Leitura dos botões NPK (lógica invertida - pull-up)
  nitrogenio_ok = !digitalRead(BOTAO_N);
  fosforo_ok = !digitalRead(BOTAO_P);
  potassio_ok = !digitalRead(BOTAO_K);
  
  // Leitura do LDR como pH (0-4095 mapeado para 0-14)
  int ldr_valor = analogRead(LDR_PIN);
  ph_atual = map(ldr_valor, 0, 4095, 0, 140) / 10.0; // Converte para 0-14 com decimal
  
  // Leitura da umidade do DHT22
  umidade_atual = dht.readHumidity();
  
  // Verifica se a leitura do DHT22 falhou
  if (isnan(umidade_atual)) {
    Serial.println("Erro na leitura do DHT22!");
    umidade_atual = 0;
  }
}

// ===== LÓGICA DE DECISÃO PARA IRRIGAÇÃO =====
void processarLogicaIrrigacao() {
  bool nutrientes_adequados = nitrogenio_ok && fosforo_ok && potassio_ok;
  bool ph_adequado = (ph_atual >= PH_MIN && ph_atual <= PH_MAX);
  bool umidade_baixa = (umidade_atual < UMIDADE_MIN);
  bool umidade_alta = (umidade_atual > UMIDADE_MAX);
  
  // DECISÃO DE IRRIGAÇÃO:
  // Liga bomba SE: Nutrientes OK + pH adequado + umidade baixa
  // Desliga bomba SE: Umidade alta OU condições inadequadas
  
  bool deve_irrigar = nutrientes_adequados && ph_adequado && umidade_baixa;
  bool deve_parar = umidade_alta || !nutrientes_adequados || !ph_adequado;
  
  if (deve_irrigar && !bomba_ligada) {
    ligarBomba();
  } else if (deve_parar && bomba_ligada) {
    desligarBomba();
  }
}

// ===== CONTROLE DA BOMBA =====
void ligarBomba() {
  digitalWrite(RELAY_PIN, HIGH);
  bomba_ligada = true;
  Serial.println("🚰 BOMBA LIGADA - Iniciando irrigação");
}

void desligarBomba() {
  digitalWrite(RELAY_PIN, LOW);
  bomba_ligada = false;
  Serial.println("⏹️  BOMBA DESLIGADA - Parando irrigação");
}

// ===== INDICAÇÃO VISUAL (LED) =====
void piscaLedStatus() {
  static unsigned long ultimo_pisca = 0;
  static bool led_estado = false;
  
  // LED pisca mais rápido quando bomba está ligada
  unsigned long intervalo = bomba_ligada ? 250 : 1000;
  
  if (millis() - ultimo_pisca >= intervalo) {
    led_estado = !led_estado;
    digitalWrite(LED_PIN, led_estado);
    ultimo_pisca = millis();
  }
}

// ===== EXIBIÇÃO DE STATUS NO MONITOR SERIAL =====
void exibirStatus() {
  Serial.println("\n📊 === STATUS DOS SENSORES ===");
  
  // NPK Status
  Serial.print("🌱 NPK: ");
  Serial.print("N="); Serial.print(nitrogenio_ok ? "✅" : "❌");
  Serial.print(" P="); Serial.print(fosforo_ok ? "✅" : "❌");  
  Serial.print(" K="); Serial.print(potassio_ok ? "✅" : "❌");
  
  bool npk_ok = nitrogenio_ok && fosforo_ok && potassio_ok;
  Serial.println(npk_ok ? " [ADEQUADO]" : " [INADEQUADO]");
  
  // pH Status
  Serial.print("🧪 pH: ");
  Serial.print(ph_atual, 1);
  bool ph_ok = (ph_atual >= PH_MIN && ph_atual <= PH_MAX);
  Serial.print(ph_ok ? " [ADEQUADO: " : " [INADEQUADO - Ideal: ");
  Serial.print(PH_MIN, 1); Serial.print("-"); Serial.print(PH_MAX, 1); Serial.println("]");
  
  // Umidade Status
  Serial.print("💧 Umidade: ");
  Serial.print(umidade_atual, 1); Serial.print("%");
  if (umidade_atual < UMIDADE_MIN) {
    Serial.println(" [BAIXA - Precisa irrigar]");
  } else if (umidade_atual > UMIDADE_MAX) {
    Serial.println(" [ALTA - Parar irrigação]");
  } else {
    Serial.println(" [ADEQUADA]");
  }
  
  // Status da Bomba
  Serial.print("🚰 Bomba d'água: ");
  Serial.println(bomba_ligada ? "LIGADA 🟢" : "DESLIGADA 🔴");
  
  // Recomendação
  Serial.print("💡 Recomendação: ");
  if (npk_ok && ph_ok && umidade_atual >= UMIDADE_MIN && umidade_atual <= UMIDADE_MAX) {
    Serial.println("Condições ideais para o tomate! 🍅");
  } else if (!npk_ok) {
    Serial.println("Verificar nutrientes do solo");
  } else if (!ph_ok) {
    Serial.println("Ajustar pH do solo");
  } else if (umidade_atual < UMIDADE_MIN) {
    Serial.println("Aumentar irrigação");
  } else if (umidade_atual > UMIDADE_MAX) {
    Serial.println("Reduzir irrigação");
  }
  
  Serial.println("=====================================");
}

// ===== FUNÇÕES AUXILIARES =====

// Função para teste manual (opcional)
void testeManual() {
  // Esta função pode ser chamada para testes específicos
  Serial.println("Executando teste manual...");
  
  // Força ligar bomba por 5 segundos
  ligarBomba();
  delay(5000);
  desligarBomba();
  
  Serial.println("Teste concluído!");
}

// Função para obter diagnóstico completo
void diagnosticoCompleto() {
  Serial.println("\n🔍 === DIAGNÓSTICO COMPLETO ===");
  Serial.print("Leitura bruta LDR: "); Serial.println(analogRead(LDR_PIN));
  Serial.print("Temperatura: "); Serial.print(dht.readTemperature()); Serial.println("°C");
  Serial.print("Sistema ativo há: "); Serial.print(millis()/1000); Serial.println(" segundos");
  Serial.println("==============================");
}