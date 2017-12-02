#include <Ultrasonic.h>
#include <LiquidCrystal.h>
#include <Key.h>
#include <Keypad.h>


#include <PubSubClient.h>
#include <SPI.h>
#include <UIPEthernet.h>
#include <utility/logging.h>
#include <RestClient.h>

const int portaBotao = 25;
int estadoBotao = 0;

const int portaTriggerSensor = A9;
const int portaEchoSensor = A8;
Ultrasonic ultrasonic(portaTriggerSensor,portaEchoSensor);

const int portaBuzzer = 24;

const int rs = 9, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte ROWS = 4;
const byte COLS = 3;
char keys [ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {5, 4, 3, A0};
byte colPins [COLS] = {8, 7, 6};

Keypad keypad = Keypad (makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ==========================================================
const byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xF1, 0x77 };
EthernetClient ethclient;

RestClient client = RestClient("192.168.3.41", 3000, ethclient);

#define SMS_TWILIO_SID "TWILIO_SID"
#define SMS_TWILIO_TOKEN "TWILIO_TOKEN"
#define SMS_PHONE_TO "1234567890"
#define SMS_PHONE_FROM "1234567890"
#define SMS_MESSAGE "Mensagem Legal"

const char* parametros = "sid=" SMS_TWILIO_SID "&token=" SMS_TWILIO_TOKEN "&to=" SMS_PHONE_TO "&from=" SMS_PHONE_FROM "&body=" SMS_MESSAGE;

#define RESPONSE_SIZE 30
char response[RESPONSE_SIZE] = {};
// ==========================================================

int estadoAlarme = 0;
int presencaDetectada = 0;
int invasorDetectado = 0;
unsigned long tempoInicial = 0;
String senhaDigitada;
bool digitandoSenha = false;
String senha = "123456";

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
      if (now - tempoInicial >= 15000 && invasorDetectado == 0) {
        Serial.println("Invasor detectado");
        invasorDetectado = 1;
      }
  }

  if (invasorDetectado == 1) {
    Serial.println("Soa o alarme");
    tone(portaBuzzer, 3000);
  } else {
    noTone(portaBuzzer);
  }
  if (estadoAlarme == 1) {
    char key = keypad.getKey();
    
    if (key && digitandoSenha == true) {
      if (key == '#') {
        digitandoSenha = false;
        lcd.clear();
        if (senhaDigitada == senha) {
          Serial.println("Senha valida");
          lcd.print("Senha valida");
          delay(1000);
          lcd.clear();
          lcd.print("Destravado");

          estadoBotao = 0;
          estadoAlarme = 0;
          presencaDetectada = 0;
          invasorDetectado = 0;
          tempoInicial = 0;
          
        } else {
          Serial.println("Senha invalida");
          lcd.print("Senha invalida");
          delay(1000);
          lcd.clear();
          lcd.print("Protegido");
          invasorDetectado = 1;
        }
      } else {
        lcd.print("*");
        senhaDigitada += String(key);
        Serial.println(senhaDigitada);
      }
    }
    
    if (key == '*') {
      digitandoSenha = true;
      senhaDigitada = "";

      lcd.clear();
      lcd.print("Digitando senha");
      lcd.setCursor(0, 1);
    }
  }
}
