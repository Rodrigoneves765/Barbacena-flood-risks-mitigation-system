/*
 *  Projeto: Pontilhão Inteligente v1.1
 *  Local: Barbacena, MG
 *  Placa: Heltec WiFi LoRa 32 (V3)
 *  Sensor: JSN-SR04T (ultrassônico impermeável)
 *  Calibração do balde: 46 cm de profundidade total
 *
 *  ATENÇÃO (hardware): o Echo do JSN-SR04T sai em 5 V. O ESP32-S3 da
 *  Heltec V3 aceita no máximo 3,3 V nos GPIOs. Use um divisor de tensão
 *  no pino Echo (ex.: 1 kΩ em série + 2 kΩ para o GND) ou um level shifter.
 */

#include <Wire.h>
#include "SSD1306Wire.h"

// --- Pinos do sensor (confira no pinout da sua V3 se estão disponíveis) ---
#define PIN_TRIG 12
#define PIN_ECHO 13

// --- Calibração hidráulica (distância lida pelo sensor, em cm) ---
const int PROFUNDIDADE_TOTAL = 46;  // balde seco
const int LIMITE_ATENCAO     = 38;  // água subiu 8 cm
const int LIMITE_ALERTA      = 30;  // água subiu 16 cm
const int LIMITE_PERIGO      = 24;  // limite da zona cega

// --- Parâmetros de medição ---
const int           N_AMOSTRAS          = 5;      // leituras por ciclo
const int           MIN_VALIDAS         = 3;      // mínimo de leituras válidas para confiar
const int           INTERVALO_AMOSTRA   = 60;     // ms entre leituras (evita eco residual)
const unsigned long TIMEOUT_ECHO_US     = 6000;   // ~100 cm de alcance, sobra para o balde
const int           FALHAS_PARA_ALARME  = 3;      // ciclos ruins seguidos até declarar falha
const float         VELOCIDADE_SOM      = 0.0343; // cm/us (~20 °C)

// --- Estados do sistema ---
enum Estado { NORMAL, ATENCAO, ALERTA, EMERGENCIA, FALHA_SENSOR };

const char* NOMES[] = { "NORMAL", "ATENCAO", "ALERTA", "EMERGENCIA", "SENSOR FALHOU" };

// --- Display OLED integrado da Heltec V3 ---
SSD1306Wire display(0x3c, 17, 18); 

Estado ultimoEstadoValido = NORMAL;
Estado estadoAtual        = NORMAL;
int    falhasSeguidas     = 0;

// A Heltec V3 alimenta o OLED pelo pino Vext (nível LOW = ligado)
void vextLigar() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(100);
}

// Uma leitura. Retorna a distância em cm, ou -1 se for inválida (sem eco).
float leituraUnica() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duracao = pulseIn(PIN_ECHO, HIGH, TIMEOUT_ECHO_US);
  if (duracao == 0) return -1;  // timeout: não houve eco

  float d = duracao * VELOCIDADE_SOM / 2.0;
  if (d < 2.0) return -1;                          // valor impossível para o sensor
  if (d > PROFUNDIDADE_TOTAL) d = PROFUNDIDADE_TOTAL;  // além do fundo = balde seco
  return d;
}

// Faz N leituras e devolve a mediana das válidas.
// Retorna false se houver menos de MIN_VALIDAS leituras boas.
bool medirDistancia(float &distancia, int &validas) {
  float v[N_AMOSTRAS];
  validas = 0;

  for (int i = 0; i < N_AMOSTRAS; i++) {
    float d = leituraUnica();
    if (d > 0) v[validas++] = d;
    delay(INTERVALO_AMOSTRA);
  }

  if (validas < MIN_VALIDAS) return false;

  // Ordenação simples (vetor pequeno)
  for (int i = 1; i < validas; i++) {
    float chave = v[i];
    int j = i - 1;
    while (j >= 0 && v[j] > chave) { v[j + 1] = v[j]; j--; }
    v[j + 1] = chave;
  }

  distancia = v[validas / 2];  // mediana
  return true;
}

Estado classificar(float d) {
  if (d > LIMITE_ATENCAO) return NORMAL;
  if (d > LIMITE_ALERTA)  return ATENCAO;
  if (d > LIMITE_PERIGO)  return ALERTA;
  return EMERGENCIA;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  vextLigar();
  display.init();
  display.flipScreenVertically();

  Serial.println("--- Sistema Pontilhao Inteligente Inicializado ---");
}

void loop() {
  float distancia = 0;
  int validas = 0;
  const char* detalhe = "";

  bool ok = medirDistancia(distancia, validas);

  if (ok) {
    falhasSeguidas = 0;
    estadoAtual = classificar(distancia);
    ultimoEstadoValido = estadoAtual;
  } else {
    falhasSeguidas++;

    if (ultimoEstadoValido >= ALERTA) {
      // Água já estava alta e o eco sumiu: provavelmente entrou na zona cega.
      // Falha segura: trata como emergência, não como "seco".
      estadoAtual = EMERGENCIA;
      detalhe = "Sem eco: zona cega?";
    } else if (falhasSeguidas >= FALHAS_PARA_ALARME) {
      estadoAtual = FALHA_SENSOR;
      detalhe = "Verifique o sensor";
    }
    // Falha isolada em NORMAL/ATENCAO: mantém o estado anterior.
  }

  // --- Display ---
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  display.setFont(ArialMT_Plain_16);
  if (ok) display.drawString(0, 0, "Dist: " + String((int)(distancia + 0.5)) + " cm");
  else    display.drawString(0, 0, "Dist: --");

  display.drawString(0, 24, NOMES[estadoAtual]);

  display.setFont(ArialMT_Plain_10);
  if (ok) display.drawString(0, 50, "Leituras validas: " + String(validas) + "/" + String(N_AMOSTRAS));
  else    display.drawString(0, 50, detalhe);

  display.display();

  // --- Serial ---
  if (ok) {
    Serial.println("Status: " + String(NOMES[estadoAtual]) +
                   " | Distancia: " + String(distancia, 1) + " cm" +
                   " | Validas: " + String(validas) + "/" + String(N_AMOSTRAS));
  } else {
    Serial.println("Status: " + String(NOMES[estadoAtual]) +
                   " | Leitura invalida (falhas seguidas: " + String(falhasSeguidas) + ") " + detalhe);
  }

  delay(700);  // as 5 leituras já levam ~300 ms, fechando ~1 s por ciclo
}
