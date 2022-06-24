/**********************************************************************************
//IoT - Automação Residencial
//Dispositivo : ESP32
//MQTT
//Red-Node/Nora/Google Assistant
/WatchDog
//Autor : Robson Brasil
//Versão : 55
//Última Modificação : 23/06/2022
 **********************************************************************************/

#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ESP32Ping.h>
#include "esp_sntp.h"
  
// Relays
#define RelayPin1 23  //D23 Ligados ao Nora
#define RelayPin2 22  //D22 Ligados ao Nora
#define RelayPin3 21  //D21 Ligados ao Nora
#define RelayPin4 19  //D19 Ligados ao Nora
#define RelayPin5 18  //D18 Ligados ao Nora
#define RelayPin6  5  //D5  Somente por MQTT
#define RelayPin7 25  //D25 Somente por MQTT
#define RelayPin8 26  //D26 Somente por MQTT

//DHT11 for reading temperature and humidity value
#define DHTPIN    16

// Uncomment whatever type you're using!

#define DHTTYPE DHT22      // DHT 22, AM2302, AM2321

//WiFi Status LED
#define wifiLed 0  //D0

//Tópicos do Subscribe
#define sub1 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor1"   // Ligados ao Nora
#define sub2 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor2"   // Ligados ao Nora
#define sub3 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor3"   // Ligados ao Nora
#define sub4 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor4"   // Ligados ao Nora
#define sub5 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor5"   // Ligados ao Nora
#define sub6 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor6"   // Somente por MQTT
#define sub7 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor7"   // Somente por MQTT
#define sub8 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor8"   // Somente por MQTT

//Tópicos do Publish
#define pub9  "ESP32-MinhaCasa/QuartoRobson/Temperatura"
#define pub10 "ESP32-MinhaCasa/QuartoRobson/Umidade"

#define MSG_BUFFER_SIZE (800)

// Constantes -------------------------------------------
const char*   ntpServer           = "pool.ntp.br";
const long    gmtOffset_sec       = -4 * 60 * 60;                   //-3h*60min*60s = -10800s
const int     daylightOffset_sec  = 0;                              //Fuso em horário de verão
const char*   ssid                = "RVR 2,4GHz";                   //WiFI Name
const char*   password            = "RodrigoValRobson2021";         //WiFi Password
//const char* mqttServer          = "broker.hivemq.com";
const char*   mqttServer          = "192.168.15.30";                //IP do Broker
const char*   mqttUserName        = "Robson Brasil";                //MQTT UserName
const char*   mqttPwd             = "LoboAlfa";                     //MQTT Password
const char*   clientID            = "ESP32ClientGoogleAssistant";   //Client ID Obs.: Deve ser único

//Configuração do IP Estático
IPAddress staticIP    (192, 168, 15, 50);
IPAddress gateway     (192, 168, 15, 1);
IPAddress subnet      (255, 255, 255, 0);
IPAddress dns         (192, 168, 15, 1);

// Variáveis globais ------------------------------------
time_t        nextNTPSync         = 0;
DHT dht(DHTPIN, DHTTYPE);

// Funções auxiliares -----------------------------------
String dateTimeStr(time_t t, int8_t tz = -4) {

// Formata time_t como "aaaa-mm-dd hh:mm:ss"
  if (t == -4) {
    return "N/D";
  } else {
    t += tz * 3600;   //Ajusta fuso horário
    struct tm *ptm;
    ptm = gmtime(&t);
    String s;
    s = ptm->tm_year + 1900;
    s += "-";
    if (ptm->tm_mon < 9) {
      s += "0";
    }
    s += ptm->tm_mon + 1;
    s += "-";
    if (ptm->tm_mday < 10) {
      s += "0";
    }
    s += ptm->tm_mday;
    s += " ";
    if (ptm->tm_hour < 10) {
      s += "0";
    }
    s += ptm->tm_hour;
    s += ":";
    if (ptm->tm_min < 10) {
      s += "0";
    }
    s += ptm->tm_min;
    s += ":";
    if (ptm->tm_sec < 10) {
      s += "0";
    }
    s += ptm->tm_sec;
    return s;
  }
}

String timeStatus() {
//Obtém o status da sinronização
  if (nextNTPSync == 0) {
    return "não definida";
  } else if (time(NULL) < nextNTPSync) {
    return "Hora Atualizada";
  } else {
    return "Atualização Pendente";
  }
}

// Callback de sincronização
void ntpSync_cb(struct timeval *tv) {

time_t t;
t = time(NULL);
//Data/Hora da próxima atualização
nextNTPSync = t + (SNTP_UPDATE_DELAY / 15000) + 1;
      Serial.println("Sincronizou com NTP em " + dateTimeStr(t));
      Serial.println();
      Serial.println("Limite para próxima sincronização é " + dateTimeStr(nextNTPSync));
}

WiFiClient espClient;
PubSubClient client(espClient);

char str_hum_data[10];
char str_temp_data[10];

unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;

void reconnect() {
    while (!client.connected()) {
  if (client.connect(clientID, mqttUserName, mqttPwd)) {
      Serial.println("MQTT Conectado");
      // ... and resubscribe
      client.subscribe(sub1);
      client.subscribe(sub2);
      client.subscribe(sub3);
      client.subscribe(sub4);
      client.subscribe(sub5);
      client.subscribe(sub6);
      client.subscribe(sub7);
      client.subscribe(sub8);
      Serial.println("Topic Subscribed");
    } 
    else {
      Serial.print("Falha, Reconectando=");
      Serial.print(client.state());
      Serial.println(" Tentando depois de 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
      Serial.print("Mensagem Chegou [");
      Serial.print(topic);
      Serial.print("] ");
  String data = "";

  if (strstr(topic, sub1)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin1, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin1, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } 
    else if (strstr(topic, sub2)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin2, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin2, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } 
    else if (strstr(topic, sub3)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin3, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin3, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } 
    else if (strstr(topic, sub4)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin4, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin4, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } 
    else if (strstr(topic, sub5)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin5, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin5, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } 
    else if (strstr(topic, sub6)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin6, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin6, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } 
    else if (strstr(topic, sub7)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin7, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin7, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } 
    else if (strstr(topic, sub8)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
      Serial.println();
    // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
      digitalWrite(RelayPin8, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } 
    else {
      digitalWrite(RelayPin8, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } 
    else {
      Serial.println("Não Inscrito no Tópico do MQTT");
  }
}

void setup() {
      Serial.begin(115200);
//Setup de atualização da Data e Hora
sntp_set_time_sync_notification_cb(ntpSync_cb);
sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
sntp_set_sync_interval(15000);
      Serial.printf("\n\nNTP sincroniza a cada %d segundos\n", SNTP_UPDATE_DELAY / 1000);

//Função para inicializar o cliente NTP
configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

//Conecta WiFi
      WiFi.begin(ssid, password);
      Serial.println("\nConectando WiFi " + String(ssid));
  if  (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
      Serial.println("Configuração Falhou");
  }
    while(WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
      Serial.println();

// Pronto
      Serial.println("");
      Serial.println("WiFi Conectado");
      Serial.println("Endereço de IP");
      Serial.println(WiFi.localIP());
      Serial.print("Subnet Mask: ");
      Serial.println(WiFi.subnetMask());
      Serial.print("Gateway IP: ");
      Serial.println(WiFi.gatewayIP());
      Serial.print("DNS 1: ");
      Serial.println(WiFi.dnsIP(0));
      Serial.print("DNS 2: ");
      Serial.println(WiFi.dnsIP(1));

pinMode(RelayPin1, OUTPUT);
pinMode(RelayPin2, OUTPUT);
pinMode(RelayPin3, OUTPUT);
pinMode(RelayPin4, OUTPUT);
pinMode(RelayPin5, OUTPUT);
pinMode(RelayPin6, OUTPUT);
pinMode(RelayPin7, OUTPUT);
pinMode(RelayPin8, OUTPUT);

pinMode(wifiLed, OUTPUT);

//Durante a partida, todos os Relés iniciam desligados
digitalWrite(RelayPin1, HIGH);
digitalWrite(RelayPin2, HIGH);
digitalWrite(RelayPin3, HIGH);
digitalWrite(RelayPin4, HIGH);
digitalWrite(RelayPin5, HIGH);
digitalWrite(RelayPin6, HIGH);
digitalWrite(RelayPin7, HIGH);
digitalWrite(RelayPin8, HIGH);

//Durante a partida o LED WiFI, inicia desligado
digitalWrite(wifiLed, HIGH);

dht.begin();

client.setServer(mqttServer, 1883);
client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
      digitalWrite(wifiLed, HIGH);
      reconnect();
   }   
   else {
      digitalWrite(wifiLed, LOW);
  }
  
      client.loop();
   
unsigned long now = millis();
  if (now - lastMsg > 1000) {

float temp_data = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
dtostrf(temp_data, 4, 2, str_temp_data);

float hum_data = dht.readHumidity();
/* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
dtostrf(hum_data, 4, 2, str_hum_data);
    
lastMsg = now;
      
      Serial.println(dateTimeStr(time(NULL)) + "\tStatus: " + timeStatus());
      Serial.print("Publish no Broker MQTT: ");
      Serial.print("Temperatura - "); Serial.print(str_temp_data); Serial.println(F("°C"));
      client.publish("ESP32-MinhaCasa/QuartoRobson/Temperatura", str_temp_data);
    
      Serial.print("Publish no Broker MQTT: ");
      Serial.print("Umidade - "); Serial.print(str_hum_data); Serial.println(F("%"));
      client.publish("ESP32-MinhaCasa/QuartoRobson/Umidade", str_hum_data);
      Serial.println();
      Serial.print("Task1 running on core ");
      Serial.println(xPortGetCoreID());
          }
}
