// INCLUSÃO DE BIBLIOTECAS
#include <Wire.h>
#include "RTClib.h"
#include "HX711.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Pushbutton.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

int BTN = 32;

Pushbutton button(BTN);

long int cont = 0;

unsigned long previousmillis = 0;

const long intervalo = 10; // Precisão Leitura Dados milissegundos

String dir = "";
String filedir = "";
int pasta = 1;

int LED = 2;
bool estado = false;
///////////////////////// DADOS DO MÓDULO RTC //////////////////////////////

RTC_DS3231 rtc; // OBJETO DO TIPO RTC_DS3231

///////////////////////// DADOS DO MÓDULO SDCard //////////////////////////////

int CS = 5;

///////////////////////// DADOS DA CÉLULA DE CARGA //////////////////////////////

#define CELULA_DT 26
#define CELULA_SCK 27

HX711 escala;

float fator_calib = 260006; // Coloque aqui o valor encontrado na calibração

void writeFile(fs::FS &fs, String path, String message)
{
  //Serial.print("Writing file: " + path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("\nFailed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    //Serial.println("\nFile written");
    digitalWrite(LED, HIGH);
  }
  else
  {
    Serial.println("\nWrite failed");
    digitalWrite(LED, LOW);
  }
  file.close();
}

void appendFile(fs::FS &fs, String path, String message)
{
  //Serial.print("Appending to file: " + path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("\nFailed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    //Serial.println("\nMessage appended");
    digitalWrite(LED, HIGH);
  }
  else
  {
    Serial.println("\nAppend failed");
    digitalWrite(LED, LOW);
  }
  file.close();
}

void createDir(fs::FS &fs, String path) {
  Serial.print("Creating Dir: " + path);
  if (fs.mkdir(path)) {
    Serial.println("\nDir created");
  } else {
    Serial.println("\nmkdir failed");
  }
}

String data()
{

  DateTime now = rtc.now();
  String data = "";

  data.concat(String(now.day(), DEC));
  data.concat('-');
  data.concat(String(now.month(), DEC));
  data.concat('-');
  data.concat(String(now.year(), DEC));
  data.concat(';');
  data.concat(String(now.hour(), DEC));
  data.concat('-');
  data.concat(String(now.minute(), DEC));
  data.concat('-');
  data.concat(String(now.second(), DEC));

  return data;
}

String dataa()
{

  DateTime now = rtc.now();
  String data = "";

  data.concat(String(now.day(), DEC));
  data.concat('/');
  data.concat(String(now.month(), DEC));
  data.concat('/');
  data.concat(String(now.year(), DEC));
  data.concat(';');
  data.concat(String(now.hour(), DEC));
  data.concat(':');
  data.concat(String(now.minute(), DEC));
  data.concat(':');
  data.concat(String(now.second(), DEC));

  return data;
}

void setup()
{
  Serial.begin(9600);
  SerialBT.begin("Teste-Estatico");
  pinMode(BTN, INPUT);
  pinMode(LED, OUTPUT);

  ///////////////////////// DADOS DO MÓDULO RTC //////////////////////////////

  if (!rtc.begin())
  { // SE O RTC NÃO FOR INICIALIZADO, FAZ

    Serial.println("DS3231 não encontrado"); // IMPRIME O TEXTO NO MONITOR SERIAL
    while (1)
      ; // SEMPRE ENTRE NO LOOP
  }
  if (rtc.lostPower())
  { // SE RTC FOI LIGADO PELA PRIMEIRA VEZ / FICOU SEM ENERGIA / ESGOTOU A BATERIA, FAZ
    Serial.println("DS3231 OK!"); // IMPRIME O TEXTO NO MONITOR SERIAL

    // REMOVA O COMENTÁRIO DE UMA DAS LINHAS ABAIXO PARA INSERIR AS INFORMAÇÕES ATUALIZADAS EM SEU RTC
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2023, 10, 2, 17, 36, 00)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
  delay(100); // INTERVALO DE 100 MILISSEGUNDOS

  ///////////////////////// DADOS DO MÓDULO SDCard //////////////////////////////

  if (!SD.begin(CS))
  {
    Serial.println("Falha no SDcard");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("Nenhum Cartao conectado");
    return;
  }

  String mensagemSetup = "\n";

  Serial.println("Escrevendo Data no Arquivo Dados.txt"); // Imprime na tela
  dir = "/";
  dir.concat(data());
  createDir(SD, dir);
  filedir = "";
  filedir = dir + "/Dados.txt";
  writeFile(SD, filedir, " ");

  ///////////////////////// DADOS DA CÉLULA DE CARGA //////////////////////////////

  escala.begin(CELULA_DT, CELULA_SCK);
  escala.set_scale(fator_calib); // Ajusta a escala
  escala.tare();                 // Ajusta o zero da escala
}

void loop()
{
  unsigned long currentmillis = millis();
  String leitura = "\n";

  if (button.getSingleDebouncedPress())
  {
    escala.tare();
    dir = "/";
    dir.concat(data());
    createDir(SD, dir);
    filedir = "";
    filedir = dir + "/Dados.txt";
    cont = 0;
  }
  else
  {
    if (currentmillis - previousmillis >= intervalo)
    {

      String dataAgora = dataa();

      Serial.print(dataAgora);
      SerialBT.print(dataAgora);
      leitura.concat(dataAgora);
      Serial.print(";");
      SerialBT.print(";");
      leitura.concat(";");
      float peso = escala.get_units();
      Serial.print(peso, 3);
      SerialBT.print(peso, 3);
      leitura.concat(String(peso, 3));
      Serial.print(";");
      SerialBT.print(";");
      leitura.concat(";");
      Serial.println(cont);
      SerialBT.println(cont);
      leitura.concat(cont);
      appendFile(SD, filedir, leitura);


      cont = cont + 10;
      previousmillis = currentmillis;
    }
  }
}
