
#include <UIPEthernet.h>
#include <SPI.h>
#include <utility/logging.h>
#include <PubSubClient.h>
#include <Ultrasonic.h>

//Botão
const int Botao = 8; //botao no pino 8
int EstadoBotao = 0; //Variavel para ler o status do pushbutton
int EstadoAlarme = 0;

int TempoLimite = 0;
int tempoInicial = 0;
int presenca = 0;

//Buzzer
const int Buzzer = 9;
int valorBuzzer = 500;

// Sensor ultrasônico
Ultrasonic ultrasonic(A5, A4); // Portas A5 e A4 para trigger e echo

// Endereço fisico do arduino na rede
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xF1, 0x67 };
void callback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, "verificaSenha") == 0) {
    noTone(Buzzer);
    EstadoAlarme = 0;
    TempoLimite = 0;
    tempoInicial = 0;
    presenca = 0;
  }
}

EthernetClient ethClient;
// Cliente de conexão com o mqtt
PubSubClient client("m14.cloudmqtt.com", 14527, callback, ethClient);
int lastReconnectAttempt;

boolean reconnect() {
  Serial.println(F("reconectando..."));
  if (client.connect("projeto3", "arduino", "123")) {
    Serial.println(F("conectado"));
    client.subscribe("verificaSenha");
    client.subscribe("ativaAlarme");
    client.subscribe("presenca");
  }
  return client.connected();
}


void setup() {
  pinMode(Botao, INPUT); //Pino com botão será entrada
  pinMode(Buzzer, OUTPUT);
  Serial.begin(9600);
  Serial.println(F("iniciando...");
  Ethernet.begin(mac);
  delay(1500);
  lastReconnectAttempt = 0;
}

void loop() {

  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;

      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
    
    EstadoBotao = digitalRead(Botao);
    if (EstadoBotao == HIGH) { //Se botão estiver pressionado (HIGH)
      if (EstadoAlarme == 1) {
        Serial.println(F("alarme desligado"));
        noTone(Buzzer);
        EstadoAlarme = 0;
        TempoLimite = 0;
        presenca = 0;
        tempoInicial = 0;
      } else {
        Serial.println(F("alarme ligado"));
        delay(100);
        client.publish("ativaAlarme", "1");
        EstadoAlarme = 1;
      }
      delay(300);
    }

    if (EstadoAlarme == 1) {
      if (TempoLimite == 1) {
        Serial.println(F("Ladrao"));
        tone(Buzzer, 1000);
      }

      int distancia = ultrasonic.distanceRead();

      if ( distancia <= 10 && presenca == 0) {
        Serial.println(F("Uma presença foi detectada"));
        presenca = 1;
      }

      if (presenca == 1) {
        long now = millis();
        if (tempoInicial == 0) {
          tempoInicial = now;
        }
        if (now > 0) {
          if (now - tempoInicial > 5000 && TempoLimite == 0) {
            Serial.println(F("5 Segundos"));
            TempoLimite = 1;
          }
        }
      }
    }
  } //else client loop
}  //void loop
