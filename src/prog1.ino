/*
 * FarmTech Solutions - Sistema de Irrigação Inteligente
 * Projeto: Monitoramento NPK, pH e Umidade para controle automatizado de irrigação
 * Cultura escolhida: TOMATE (pH ideal: 6.0-6.8, Umidade: 60-80%)
 * 
 * VERSÃO COM BIBLIOTECA DHT + INTEGRAÇÃO API METEOROLÓGICA
 * 
 * Sensores:
 * - 3 Botões: Simulam sensores NPK (Nitrogênio, Fósforo, Potássio)
 * - LDR: Simula sensor de pH (0-14)
 * - DHT22: Sensor de umidade e temperatura
 * - Relay: Controla bomba d'água
 * - LED: Indica status do sistema
 * 
 * Integração:
 * - API Meteorológica (via variáveis manuais para simulação)
 * - Lógica avançada de irrigação
 * - Monitoramento completo via Serial
 */

#include <DHT.h>

// ===== DEFINIÇÕES DE PINOS =====
#define BOTAO_N    2    // Botão Nitrogênio
#define BOTAO_P    4    // Botão Fósforo  
#define BOTAO_K    5    // Botão Potássio
#define DHT_PIN    15   // Sensor DHT22
#define RELAY_PIN  18   // Módulo Relé (Bomba)
#define LED_PIN    19   // LED de Status
#define LDR_PIN    A0   // Sensor LDR (pH)

// ===== CONFIGURAÇÕES DHT =====
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
float temperatura_atual = 0.0;
bool bomba_ligada = false;

// ===== INTEGRAÇÃO API METEOROLÓGICA =====
// Estas variáveis são atualizadas manualmente com dados do Python
bool previsao_chuva = false;        // true = chuva prevista
float intensidade_chuva = 0.0;      // mm de chuva prevista
String condicao_tempo = "Limpo";    // Descrição do tempo

// ===== CONTROLE DE TEMPO =====
unsigned long ultimo_check = 0;
unsigned long ultimo_led = 0;
bool led_estado = false;
const unsigned long INTERVALO_CHECK = 3000; // Verifica a cada 3 segundos (DHT22 é lento)
const unsigned long INTERVALO_LED = 500;    // LED pisca a cada 500ms

// ===== FUNÇÃO DE SETUP =====
void setup() {
  Serial.begin(115200);
  
  // Aguarda inicialização do Serial
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("=== FarmTech Solutions - Sistema de Irrigação ===");
  Serial.println("Cultura: TOMATE");
  Serial.println("pH ideal: 6.0-6.8 | Umidade ideal: 60-80%");
  Serial.println("Versão: 2.0 com integração meteorológica");
  Serial.println("=====================================");
  
  // Configuração dos pinos
  pinMode(BOTAO_N, INPUT_PULLUP);
  pinMode(BOTAO_P, INPUT_PULLUP);  
  pinMode(BOTAO_K, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  // Inicialização
  digitalWrite(RELAY_PIN, LOW);  // Bomba desligada
  digitalWrite(LED_PIN, LOW);    // LED desligado
  
  // Inicializa DHT22
  dht.begin();
  Serial.println("DHT22 inicializado!");
  
  delay(2000); // Aguarda estabilização dos sensores
  Serial.println("Sistema inicializado com sucesso!");
  Serial.println("Aguardando primeira leitura dos sensores...");
  Serial.println("=====================================");
}

// ===== FUNÇÃO PRINCIPAL =====
void loop() {
  // Verifica sensores a cada INTERVALO_CHECK
  if (millis() - ultimo_check >= INTERVALO_CHECK) {
    lerSensores();
    lerComandosExternos();
    processarLogicaIrrigacao();
    exibirStatus();
    ultimo_check = millis();
  }
  
  // Controla LED independentemente
  controlarLED();
  
  // Pequeno delay para não sobrecarregar o processador
  delay(10);
}

// ===== LEITURA DOS SENSORES =====
void lerSensores() {
  Serial.print("📡 Lendo sensores...");
  
  // Leitura dos botões NPK (lógica invertida - pull-up)
  nitrogenio_ok = !digitalRead(BOTAO_N);
  fosforo_ok = !digitalRead(BOTAO_P);
  potassio_ok = !digitalRead(BOTAO_K);
  
  // Leitura do LDR como pH (0-4095 mapeado para 0-14)
  int ldr_valor = analogRead(LDR_PIN);
  ph_atual = map(ldr_valor, 0, 4095, 0, 140) / 10.0; // Converte para 0-14 com decimal
  
  // Leitura do DHT22
  umidade_atual = dht.readHumidity();
  temperatura_atual = dht.readTemperature();
  
  // Verifica se as leituras do DHT22 são válidas
  if (isnan(umidade_atual) || isnan(temperatura_atual)) {
    Serial.println(" ❌ Erro na leitura do DHT22!");
    
    // Usar valores anteriores ou padrão se for a primeira leitura
    if (isnan(umidade_atual)) {
      umidade_atual = 50.0; // Valor padrão
    }
    if (isnan(temperatura_atual)) {
      temperatura_atual = 25.0; // Valor padrão
    }
  } else {
    Serial.println(" ✅ OK");
  }
}

// ===== LEITURA DE COMANDOS EXTERNOS =====
void lerComandosExternos() {
  if (Serial.available()) {
    String comando = Serial.readString();
    comando.trim();
    
    if (comando == "SUSPENDER_CHUVA") {
      // Força parar irrigação por previsão de chuva
      previsao_chuva = true;
      intensidade_chuva = 5.0;
      if (bomba_ligada) {
        desligarBomba();
        Serial.println("🌧️ Irrigação suspensa - Comando de chuva recebido");
      }
    }
    else if (comando == "PERMITIR_IRRIGACAO") {
      previsao_chuva = false;
      intensidade_chuva = 0.0;
      Serial.println("✅ Irrigação normal permitida");
    }
    else if (comando == "PARADA_FORCADA") {
      previsao_chuva = true;
      intensidade_chuva = 10.0;
      desligarBomba();
      Serial.println("🚨 Parada forçada - Chuva intensa detectada");
    }
    else if (comando == "STATUS") {
      enviarDadosEstruturados();
    }
    else if (comando == "DIAGNOSTICO") {
      diagnosticoCompleto();
    }
    else if (comando == "TESTE") {
      testeManual();
    }
  }
}

// ===== LÓGICA DE DECISÃO PARA IRRIGAÇÃO =====
void processarLogicaIrrigacao() {
  bool nutrientes_adequados = nitrogenio_ok && fosforo_ok && potassio_ok;
  bool ph_adequado = (ph_atual >= PH_MIN && ph_atual <= PH_MAX);
  bool umidade_baixa = (umidade_atual < UMIDADE_MIN);
  bool umidade_alta = (umidade_atual > UMIDADE_MAX);
  
  // ⭐ NOVA LÓGICA: Considerar previsão meteorológica
  bool chuva_prevista = previsao_chuva && intensidade_chuva > 2.0;
  
  // DECISÃO DE IRRIGAÇÃO (atualizada):
  // Liga bomba SE: Nutrientes OK + pH adequado + umidade baixa + SEM chuva prevista
  bool deve_irrigar = nutrientes_adequados && 
                     ph_adequado && 
                     umidade_baixa && 
                     !chuva_prevista;  // ⭐ NÃO irrigar se vai chover
  
  // Desliga bomba SE: Umidade alta OU condições inadequadas OU chuva prevista
  bool deve_parar = umidade_alta || 
                   !nutrientes_adequados || 
                   !ph_adequado ||
                   chuva_prevista;    // ⭐ PARAR se vai chover
  
  if (deve_irrigar && !bomba_ligada) {
    ligarBomba();
  } else if (deve_parar && bomba_ligada) {
    desligarBomba();
    if (chuva_prevista) {
      Serial.println("🌧️ Irrigação suspensa - Previsão de chuva");
    }
  }
}

// ===== CONTROLE DA BOMBA =====
void ligarBomba() {
  digitalWrite(RELAY_PIN, HIGH);
  bomba_ligada = true;
  Serial.println();
  Serial.println("🚰 BOMBA LIGADA - Iniciando irrigação");
  Serial.println("💧 Água sendo fornecida para a plantação de tomate");
  Serial.print("🌡️  Condições: ");
  Serial.print(temperatura_atual, 1);
  Serial.println("°C");
}

void desligarBomba() {
  digitalWrite(RELAY_PIN, LOW);
  bomba_ligada = false;
  Serial.println();
  Serial.println("⏹️  BOMBA DESLIGADA - Parando irrigação");
  Serial.println("✋ Irrigação interrompida");
}

// ===== CONTROLE DO LED =====
void controlarLED() {
  unsigned long intervalo;
  
  if (bomba_ligada) {
    intervalo = 200;  // Pisca rápido quando irrigando
  } else if (previsao_chuva) {
    intervalo = 100;  // Pisca muito rápido se há chuva prevista
  } else {
    intervalo = 1000; // Pisca lento em condições normais
  }
  
  if (millis() - ultimo_led >= intervalo) {
    led_estado = !led_estado;
    digitalWrite(LED_PIN, led_estado);
    ultimo_led = millis();
  }
}

// ===== EXIBIÇÃO DE STATUS NO MONITOR SERIAL =====
void exibirStatus() {
  Serial.println();
  Serial.println("📊 === STATUS DOS SENSORES ===");
  
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
  Serial.print(ph_ok ? " [ADEQUADO]" : " [INADEQUADO - Ideal: ");
  if (!ph_ok) {
    Serial.print(PH_MIN, 1); Serial.print("-"); Serial.print(PH_MAX, 1);
  }
  Serial.println("]");
  
  // Umidade e Temperatura (DHT22)
  Serial.print("💧 Umidade: ");
  Serial.print(umidade_atual, 1); Serial.print("%");
  if (umidade_atual < UMIDADE_MIN) {
    Serial.println(" [BAIXA - Precisa irrigar]");
  } else if (umidade_atual > UMIDADE_MAX) {
    Serial.println(" [ALTA - Parar irrigação]");
  } else {
    Serial.println(" [ADEQUADA]");
  }
  
  Serial.print("🌡️  Temperatura: ");
  Serial.print(temperatura_atual, 1);
  Serial.println("°C");
  
  // ⭐ NOVO: Status Meteorológico
  Serial.print("🌤️  Previsão Meteorológica: ");
  if (previsao_chuva) {
    Serial.print("CHUVA PREVISTA (");
    Serial.print(intensidade_chuva, 1);
    Serial.print("mm) - ");
    Serial.println(condicao_tempo);
  } else {
    Serial.print("SEM CHUVA - ");
    Serial.println(condicao_tempo);
  }
  
  // Status da Bomba
  Serial.print("🚰 Bomba d'água: ");
  Serial.println(bomba_ligada ? "LIGADA 🟢" : "DESLIGADA 🔴");
  
  // Diagnóstico técnico
  Serial.print("🔧 Valores brutos - LDR: ");
  Serial.print(analogRead(LDR_PIN));
  Serial.print(" | RAM livre: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  
  // Recomendação
  Serial.print("💡 Recomendação: ");
  if (npk_ok && ph_ok && umidade_atual >= UMIDADE_MIN && umidade_atual <= UMIDADE_MAX && !previsao_chuva) {
    Serial.println("Condições ideais para o tomate! 🍅");
  } else if (!npk_ok) {
    Serial.println("Verificar nutrientes do solo (pressione botões NPK)");
  } else if (!ph_ok) {
    Serial.println("Ajustar pH do solo (mova o LDR)");
  } else if (umidade_atual < UMIDADE_MIN && !previsao_chuva) {
    Serial.println("Aumentar irrigação");
  } else if (umidade_atual > UMIDADE_MAX) {
    Serial.println("Reduzir irrigação");
  } else if (previsao_chuva) {
    Serial.println("Aguardar chuva - irrigação suspensa");
  }
  
  Serial.println("=====================================");
}

// ===== ENVIO DE DADOS ESTRUTURADOS =====
void enviarDadosEstruturados() {
  // Formato CSV para facilitar análise em R/Python
  Serial.println("TIMESTAMP,N,P,K,PH,UMIDADE,TEMPERATURA,BOMBA,CHUVA,INTENSIDADE");
  Serial.print(millis()/1000); Serial.print(",");
  Serial.print(nitrogenio_ok ? 1 : 0); Serial.print(",");
  Serial.print(fosforo_ok ? 1 : 0); Serial.print(",");
  Serial.print(potassio_ok ? 1 : 0); Serial.print(",");
  Serial.print(ph_atual, 2); Serial.print(",");
  Serial.print(umidade_atual, 2); Serial.print(",");
  Serial.print(temperatura_atual, 2); Serial.print(",");
  Serial.print(bomba_ligada ? 1 : 0); Serial.print(",");
  Serial.print(previsao_chuva ? 1 : 0); Serial.print(",");
  Serial.println(intensidade_chuva, 2);
}

// ===== FUNÇÃO PARA TESTE MANUAL =====
void testeManual() {
  Serial.println("🧪 Iniciando teste completo do sistema...");
  
  // Salvar estado atual
  bool bomba_anterior = bomba_ligada;
  
  // Teste da bomba
  Serial.println("Testando bomba por 3 segundos...");
  ligarBomba();
  delay(3000);
  desligarBomba();
  
  // Teste do LED
  Serial.println("Testando LED...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  // Restaurar estado anterior
  if (bomba_anterior) {
    ligarBomba();
  }
  
  Serial.println("✅ Teste completo finalizado!");
}

// ===== FUNÇÃO DE DIAGNÓSTICO =====
void diagnosticoCompleto() {
  Serial.println();
  Serial.println("🔍 === DIAGNÓSTICO COMPLETO ===");
  Serial.print("⏱️  Sistema ativo há: "); 
  Serial.print(millis()/1000); 
  Serial.println(" segundos");
  
  Serial.print("📊 Leitura bruta LDR: "); 
  Serial.println(analogRead(LDR_PIN));
  
  Serial.print("🔘 Estados dos botões - N: ");
  Serial.print(!digitalRead(BOTAO_N) ? "PRESS" : "SOLTO");
  Serial.print(" | P: ");
  Serial.print(!digitalRead(BOTAO_P) ? "PRESS" : "SOLTO");
  Serial.print(" | K: ");
  Serial.println(!digitalRead(BOTAO_K) ? "PRESS" : "SOLTO");
  
  Serial.print("💾 RAM livre: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  
  Serial.print("🌡️  Temperatura DHT22: ");
  Serial.print(dht.readTemperature());
  Serial.println("°C");
  
  Serial.print("💧 Umidade DHT22: ");
  Serial.print(dht.readHumidity());
  Serial.println("%");
  
  Serial.print("🔧 Frequência CPU: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  
  Serial.println("==============================");
}

// ===== COMANDOS SERIAIS DISPONÍVEIS =====
/*
 * Comandos que podem ser enviados via Monitor Serial:
 * 
 * STATUS - Exibe dados em formato CSV
 * DIAGNOSTICO - Diagnóstico completo do sistema
 * TESTE - Executa teste manual de componentes
 * SUSPENDER_CHUVA - Simula comando da API (chuva prevista)
 * PERMITIR_IRRIGACAO - Remove restrição de chuva
 * PARADA_FORCADA - Para bomba imediatamente
 * 
 * Para simular integração com Python:
 * 1. Execute o script Python para obter dados meteorológicos
 * 2. Modifique as variáveis previsao_chuva e intensidade_chuva
 * 3. Recompile e teste no Wokwi
 */