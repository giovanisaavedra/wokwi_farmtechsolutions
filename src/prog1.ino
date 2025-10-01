#include <DHT.h>

#define RELAY_PIN 21
#define BTN_N 25
#define BTN_P 26
#define BTN_K 27
#define LDR_PIN 34

// Simulação no lugar do DHT22
float simularUmidade() {
  return random(30, 90); // valor fictício entre 30% e 90%
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BTN_N, INPUT_PULLDOWN);
  pinMode(BTN_P, INPUT_PULLDOWN);
  pinMode(BTN_K, INPUT_PULLDOWN);

  randomSeed(analogRead(0)); // inicializa gerador de números aleatórios
  Serial.println("===== Sistema de Irrigação Inteligente (Tomate 🍅 - SIMULAÇÃO) =====");
}

void loop() {
  bool n = digitalRead(BTN_N);
  bool p = digitalRead(BTN_P);
  bool k = digitalRead(BTN_K);

  int ldrRaw = analogRead(LDR_PIN);
  float ph = map(ldrRaw, 0, 4095, 0, 14);

  float umidade = simularUmidade();

  // SE FOSSEMOS UTILIZAR DADOS REAIS E "CRUZAR" OS DADOS PARA LIGAR A BOMBA
  //bool bomba = false;
  //if (n && p && k && (ph >= 6.0 && ph <= 6.8) && (umidade < 60)) {
  //  bomba = true;
  //}

  //VAMOS APENAS CONSIDERAR A HUMIDADE PARA SIMULAR A BOMBA LIGANDO
 bool bomba = false; 
  if (umidade < 60) {
  bomba = true; // prioridade: solo seco
} else if (n && p && k && (ph >= 6.0 && ph <= 6.8)) {
  bomba = false; // solo ok mas depende de nutrientes e pH
} else {
  bomba = false; // não precisa irrigar
}  

  digitalWrite(RELAY_PIN, bomba ? HIGH : LOW);

  Serial.println("--------------------------------------------------");
  Serial.printf("N: %s | P: %s | K: %s\n", n ? "OK" : "Baixo", p ? "OK" : "Baixo", k ? "OK" : "Baixo");
  Serial.printf("pH do solo   : %.2f (raw=%d)\n", ph, ldrRaw);
  Serial.printf("Umidade solo : %.1f %%\n", umidade);
  Serial.printf("Bomba de irrigação: %s\n", bomba ? "LIGADA 🚰" : "DESLIGADA");
  Serial.println("--------------------------------------------------\n");

  delay(5000);
}
