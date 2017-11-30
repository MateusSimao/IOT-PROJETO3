#include <Ultrasonic.h>
#include <LiquidCrystal.h>

const int portaBotao = 13;
int estadoBotao = 0;

const int portaTriggerSensor = A9;
const int portaEchoSensor = A8;
Ultrasonic ultrasonic(portaTriggerSensor,portaEchoSensor);

const int portaBuzzer = 24;

const int rs = 9, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int estadoAlarme = 0;
int presencaDetectada = 0;
int invasorDetectado = 0;
unsigned long tempoInicial = 0;

void verificaBotao()
{
  if (digitalRead(portaBotao) == 1) {
    if (estadoBotao == 0) {
      Serial.println("Alarme ligado");
      lcd.clear();
      lcd.print("Protegido");
      
      estadoBotao = 1;
      estadoAlarme = 1;
      delay(300);
    } else if (estadoBotao == 1) {
      Serial.println("Alarme desligado");
      lcd.clear();
      lcd.print("Destravado");
      
      estadoBotao = 0;
      estadoAlarme = 0;
      presencaDetectada = 0;
      invasorDetectado = 0;
      tempoInicial = 0;
      
      delay(300);
    }
  }
}

void verificaPresenca()
{
  if (ultrasonic.distanceRead() <= 20 && presencaDetectada == 0) {
    presencaDetectada = 1;
    Serial.println("Uma presença foi detectada!!!");
  }
}

void ativaSensor()
{
  if (estadoAlarme == 1) {
    verificaPresenca();
  }
}

void setup()
{
  Serial.begin(9600);
  lcd.begin(16,2);
  pinMode(portaBotao, INPUT);
  pinMode(portaBuzzer, OUTPUT);

  lcd.print("Destravado");
}

void loop()
{
  verificaBotao();
  ativaSensor();
  
  if (presencaDetectada == 1) {
    unsigned long now = millis();
    if (tempoInicial == 0) {
      tempoInicial = now;
    }
      if (now - tempoInicial >= 5000 && invasorDetectado == 0) {
        Serial.println("Invasor detectado");
        invasorDetectado = 1;
      }
  }

  if (invasorDetectado == 1) {
    Serial.println("Soa o alarme");
    tone(portaBuzzer, 1000);
  } else {
    noTone(portaBuzzer);
  }
  
}
