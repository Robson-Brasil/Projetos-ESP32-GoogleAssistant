/**********************************************************************************
IoT - Automação Residencial
Dispositivo : ESP32 WROOM32
Broker MQTT
Red-Node/Nora/Google Assistant
Autor : Robson Brasil
Versão : 4 - Alfa
Última Modificação : 20/07/2022
**********************************************************************************/

//Bibliotecas
#include <WiFi.h>         // Importa a Biblioteca WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#include "DHT.h"          // Importa a Biblioteca DHT
#include <WiFiUdp.h>      // Importa a Biblioteca WiFiUdp
#include <esp_task_wdt.h> // Importa a Biblioteca do WatchDog

//Tópicos do Subscribe
const char* sub0 = "ESP32/MinhaCasa/QuartoRobson/Ligar-DesligarTudo/Comando";  // Somente por MQTT
const char* sub1 = "ESP32/MinhaCasa/QuartoRobson/Interruptor1/Comando";   // Ligados ao Nora/MQTT
const char* sub2 = "ESP32/MinhaCasa/QuartoRobson/Interruptor2/Comando";   // Ligados ao Nora/MQTT
const char* sub3 = "ESP32/MinhaCasa/QuartoRobson/Interruptor3/Comando";   // Ligados ao Nora/MQTT
const char* sub4 = "ESP32/MinhaCasa/QuartoRobson/Interruptor4/Comando";   // Ligados ao Nora/MQTT
const char* sub5 = "ESP32/MinhaCasa/QuartoRobson/Interruptor5/Comando";   // Ligados ao Nora/MQTT
const char* sub6 = "ESP32/MinhaCasa/QuartoRobson/Interruptor6/Comando";   // Somente por MQTT
const char* sub7 = "ESP32/MinhaCasa/QuartoRobson/Interruptor7/Comando";   // Somente por MQTT
const char* sub8 = "ESP32/MinhaCasa/QuartoRobson/Interruptor8/Comando";   // Somente por MQTT

//Tópicos do Publish
#define pub0  "ESP32/MinhaCasa/QuartoRobson/Ligar-DesligarTudo/Estado"  // Somente por MQTT
#define pub1  "ESP32/MinhaCasa/QuartoRobson/Interruptor1/Estado"   // Ligados ao Nora/MQTT
#define pub2  "ESP32/MinhaCasa/QuartoRobson/Interruptor2/Estado"   // Ligados ao Nora/MQTT
#define pub3  "ESP32/MinhaCasa/QuartoRobson/Interruptor3/Estado"   // Ligados ao Nora/MQTT
#define pub4  "ESP32/MinhaCasa/QuartoRobson/Interruptor4/Estado"   // Ligados ao Nora/MQTT
#define pub5  "ESP32/MinhaCasa/QuartoRobson/Interruptor5/Estado"   // Ligados ao Nora/MQTT
#define pub6  "ESP32/MinhaCasa/QuartoRobson/Interruptor6/Estado"   // Somente por MQTT
#define pub7  "ESP32/MinhaCasa/QuartoRobson/Interruptor7/Estado"   // Somente por MQTT
#define pub8  "ESP32/MinhaCasa/QuartoRobson/Interruptor8/Estado"   // Somente por MQTT
#define pub9  "ESP32/MinhaCasa/QuartoRobson/Temperatura"
#define pub10 "ESP32/MinhaCasa/QuartoRobson/Umidade"
#define pub11 "ESP32/MinhaCasa/QuartoRobson/SensacaoTermica"


                                                   
#define ID_MQTT  "ESP32-IoT"   /* ID MQTT (para identificação de sessão)
                               IMPORTANTE: Este deve ser único no broker (ou seja, 
                               se um client MQTT tentar entrar com o mesmo 
                               ID de outro já conectado ao broker, o broker 
                               irá fechar a conexão de um deles).*/
 
//Defines - Mapeamento de pinos do NodeMCU Relays
#define RelayPin1 23  //D23 Ligados ao Nora/MQTT
#define RelayPin2 22  //D22 Ligados ao Nora/MQTT
#define RelayPin3 21  //D21 Ligados ao Nora/MQTT
#define RelayPin4 19  //D19 Ligados ao Nora/MQTT
#define RelayPin5 18  //D18 Ligados ao Nora/MQTT
#define RelayPin6  5  //D5  Somente por MQTT
#define RelayPin7 25  //D25 Somente por MQTT
#define RelayPin8 26  //D26 Somente por MQTT

//WiFi Status LED
#define wifiLed    0  //D0

int toggleState_0 = 1; //Define integer to remember the toggle state for relay 1
int toggleState_1 = 1; //Define integer to remember the toggle state for relay 1
int toggleState_2 = 1; //Define integer to remember the toggle state for relay 2
int toggleState_3 = 1; //Define integer to remember the toggle state for relay 3
int toggleState_4 = 1; //Define integer to remember the toggle state for relay 4
int toggleState_5 = 1; //Define integer to remember the toggle state for relay 1
int toggleState_6 = 1; //Define integer to remember the toggle state for relay 2
int toggleState_7 = 1; //Define integer to remember the toggle state for relay 3
int toggleState_8 = 1; //Define integer to remember the toggle state for relay 4
int status_todos = 0;

//DHT22 para leitura dos valores  de Temperatura e Umidity
#define DHTPIN    16
#define DHTTYPE DHT22      // DHT 22
DHT dht(DHTPIN, DHTTYPE);
 
//Configurações do WIFI
const char* SSID          = "RVR 2,4GHz";           // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD      = "RodrigoValRobson2021"; // Senha da rede WI-FI que deseja se conectar
  
//Configurações do Broker MQTT
const char* BROKER_MQTT   = "192.168.15.30";      // URL do broker MQTT que se deseja utilizar
const char* mqttUserName  = "RobsonBrasil";       // MQTT UserName
const char* mqttPwd       = "LoboAlfa";           // MQTT Password
int BROKER_PORT           = 1883;                 // Porta do Broker MQTT

//IP Estático
IPAddress staticIP        (192, 168, 15, 50);
IPAddress gateway         (192, 168, 15, 1);
IPAddress subnet          (255, 255, 255, 0);
IPAddress dns             (192, 168, 15, 1);

//WatchDog 
hw_timer_t *timer = NULL; //faz o controle do temporizador (interrupção por tempo)

//Função que o temporizador irá chamar, para reiniciar o ESP32
void IRAM_ATTR resetModule(){
    ets_printf("(WatchDog) Reiniciar\n"); //imprime no log
    esp_restart(); //reinicia o chip
  }
  
void watchDogRefresh()
{
  timerWrite(timer, 0);      //reset timer (feed watchdog)
}
 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

char str_hum_data[10];
char str_temp_data[10];
char str_tempterm_data[10];
char str_tempF_data[10];

#define MSG_BUFFER_SIZE (1000)
unsigned long lastMsg = 0;
int value = 0;
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);
void watchDogRefresh();
void IRAM_ATTR watchDogInterrupt();

/* 
Implementações das funções
 */
void setup() 
{
    //inicializações:
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
    dht.begin();

    //WatchDog 
    //hw_timer_t * timerBegin(uint8_t num, uint16_t divider, bool countUp)
    /*
      num: é a ordem do temporizador. Podemos ter quatro temporizadores, então a ordem pode ser [0,1,2,3].
      divider: É um prescaler (reduz a frequencia por fator). Para fazer um agendador de um segundo, 
      usaremos o divider como 80 (clock principal do ESP32 é 80MHz). Cada instante será T = 1/(80) = 1us
      countUp: True o contador será progressivo
    */
    timer = timerBegin(0, 80, true); //timerID 0, div 80
    //timer, callback, interrupção de borda
    timerAttachInterrupt(timer, &resetModule, true);
    //timer, tempo (us), repetição
    timerAlarmWrite(timer, 10000000, true);
    timerAlarmEnable(timer); //habilita a interrupção
}
  
//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
void initSerial() 
{
    Serial.begin(115200);
}
 
//Função: inicializa e conecta-se na rede WI-FI desejada
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}

//Função: inicializa parâmetros de conexão MQTT(endereço do broker, porta e seta função de callback)
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //Informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //Atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}
  
//Função: Função de callback, esta função é chamada toda vez que uma informação de um dos tópicos subescritos chega.
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    Serial.print("Mensagem enviada ao Broker MQTT no Tópico -> [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    String data = "";

    if (strstr(topic, sub0)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin1, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      digitalWrite(RelayPin2, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      digitalWrite(RelayPin3, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      digitalWrite(RelayPin4, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      digitalWrite(RelayPin5, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      digitalWrite(RelayPin6, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      digitalWrite(RelayPin7, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      digitalWrite(RelayPin8, HIGH);  // Turn the Relé on Note that HIGH is the voltage level
      status_todos = 0;
      toggleState_0 = 0;
      MQTT.publish(pub0, "0");
      } 
    else {
      digitalWrite(RelayPin1, LOW);  // Turn the Relé off by making the voltage LOW
      digitalWrite(RelayPin2, LOW);  // Turn the Relé off by making the voltage LOW
      digitalWrite(RelayPin3, LOW);  // Turn the Relé off by making the voltage LOW
      digitalWrite(RelayPin4, LOW);  // Turn the Relé off by making the voltage LOW
      digitalWrite(RelayPin5, LOW);  // Turn the Relé off by making the voltage LOW
      digitalWrite(RelayPin6, LOW);  // Turn the Relé off by making the voltage LOW
      digitalWrite(RelayPin7, LOW);  // Turn the Relé off by making the voltage LOW
      digitalWrite(RelayPin8, LOW);  // Turn the Relé off by making the voltage LOW
      status_todos = 1;
      toggleState_0 = 1;
      MQTT.publish(pub0, "1");
       }
    }
    if (strstr(topic, sub1)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin1, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_1 = 0;
      MQTT.publish(pub1, "0");
      } 
    else {
      digitalWrite(RelayPin1, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_1 = 1;
      MQTT.publish(pub1, "1");
      }
    } 
    if (strstr(topic, sub2)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin2, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_2 = 0;
      MQTT.publish(pub2, "0");
      } 
    else {
      digitalWrite(RelayPin2, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_2 = 1;
      MQTT.publish(pub2, "1");
      }
    }
    if (strstr(topic, sub3)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin3, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_3 = 0;
      MQTT.publish(pub3, "0");
      } 
    else {
      digitalWrite(RelayPin3, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_3 = 1;
      MQTT.publish(pub3, "1");
      }
    }
    if (strstr(topic, sub4)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin4, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_4 = 0;
      MQTT.publish(pub4, "0");
      } 
    else {
      digitalWrite(RelayPin4, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_4 = 1;
      MQTT.publish(pub4, "1");
      }
    }
    if (strstr(topic, sub5)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin5, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_5 = 0;
      MQTT.publish(pub5, "0");
      } 
    else {
      digitalWrite(RelayPin5, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_5 = 1;
      MQTT.publish(pub5, "1");
      }
    }
    if (strstr(topic, sub6)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin6, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_6 = 0;
      MQTT.publish(pub6, "0");
      } 
    else {
      digitalWrite(RelayPin6, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_6 = 1;
      MQTT.publish(pub6, "1");
      }
    }
    if (strstr(topic, sub7)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin7, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_7 = 0;
      MQTT.publish(pub7, "0");
      } 
    else {
      digitalWrite(RelayPin7, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_7 = 1;
      MQTT.publish(pub7, "1");
      }
    }
    if (strstr(topic, sub8)) {
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(RelayPin8, HIGH);  // Turn the LED on (Note that LOW is the voltage level
      toggleState_8 = 0;
      MQTT.publish(pub8, "0");
      } 
    else {
      digitalWrite(RelayPin8, LOW);  // Turn the LED off by making the voltage HIGH
      toggleState_8 = 1;
      MQTT.publish(pub8, "1");
      }
   } 
}

/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.*/
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT, mqttUserName, mqttPwd)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(sub0);
            MQTT.subscribe(sub1);
            MQTT.subscribe(sub2);
            MQTT.subscribe(sub3);
            MQTT.subscribe(sub4);
            MQTT.subscribe(sub5);
            MQTT.subscribe(sub6);
            MQTT.subscribe(sub7);
            MQTT.subscribe(sub8);
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.print(MQTT.state());
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

//Função: reconecta-se ao WiFi
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
    Serial.println("\nConectando WiFi " + String(SSID));
    if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    Serial.println("Configuração Falhou");
  }     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("");
    Serial.println("WiFi Conectado");
    Serial.print("Endereço de IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway IP: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS 1: ");
    Serial.println(WiFi.dnsIP(0));
    Serial.print("DNS 2: ");
    Serial.println(WiFi.dnsIP(1));
}
 
/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
Em caso de desconexão (qualquer uma das duas), a conexão  é refeita.*/
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
        reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

//Função: inicializa o output em nível lógico baixo
void InitOutput(void)
{
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);
  pinMode(RelayPin5, OUTPUT);
  pinMode(RelayPin6, OUTPUT);
  pinMode(RelayPin7, OUTPUT);
  pinMode(RelayPin8, OUTPUT);

//Durante a partida o LED WiFI, inicia desligado
  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, HIGH);  

//Durante a partida, todos os Relés iniciam desligados
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  digitalWrite(RelayPin3, HIGH);
  digitalWrite(RelayPin4, HIGH);
  digitalWrite(RelayPin5, HIGH);
  digitalWrite(RelayPin6, HIGH);
  digitalWrite(RelayPin7, HIGH);
  digitalWrite(RelayPin8, HIGH);          
}

//Programa Principal
void loop() 
{
   
//WatchDog 
    timerWrite(timer, 0); //reseta o temporizador (alimenta o watchdog) 
    long tme = millis(); //tempo inicial do loop
    delay(100);
    tme = millis() - tme; //calcula o tempo (atual - inicial)
    watchDogRefresh();

    unsigned long now = millis();
    if (now - lastMsg > 1000) {

    float temp_data = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
    dtostrf(temp_data, 4, 2, str_temp_data);

    float hum_data = dht.readHumidity();
    /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
    dtostrf(hum_data, 4, 2, str_hum_data);

    float tempF_data = dht.readTemperature(true);
    dtostrf(tempF_data, 4, 2, str_tempF_data);

    float tempterm_data = 0;
    dtostrf(tempterm_data, 4, 2, str_tempterm_data);

    tempterm_data = dht.computeHeatIndex(tempF_data, hum_data);
    tempterm_data = dht.convertFtoC(tempterm_data);
    dtostrf(tempterm_data, 4, 2, str_tempterm_data);
    
    lastMsg = now;
      
    MQTT.publish(pub9, str_temp_data);
    
    MQTT.publish(pub10, str_hum_data);

    MQTT.publish(pub11, str_tempterm_data);

    if (digitalRead(RelayPin1) == HIGH){
      MQTT.publish(pub1, "0");
    } else {
      MQTT.publish(pub1, "1");
    }
    if (digitalRead(RelayPin2) == HIGH){
      MQTT.publish(pub2, "0");
    } else {
      MQTT.publish(pub2, "1");
    }
    if (digitalRead(RelayPin3) == HIGH){
      MQTT.publish(pub3, "0");
    } else {
      MQTT.publish(pub3, "1");
    }
    if (digitalRead(RelayPin4) == HIGH){
      MQTT.publish(pub4, "0");
    } else {
      MQTT.publish(pub4, "1");
    }
    if (digitalRead(RelayPin5) == HIGH){
      MQTT.publish(pub5, "0");
    } else {
      MQTT.publish(pub5, "1");
    }
    if (digitalRead(RelayPin6) == HIGH){
      MQTT.publish(pub6, "0");
    } else {
      MQTT.publish(pub6, "1");
    }
    if (digitalRead(RelayPin7) == HIGH){
      MQTT.publish(pub7, "0");
    } else {
      MQTT.publish(pub7, "1");
    }
    if (digitalRead(RelayPin8) == HIGH){
      MQTT.publish(pub8, "0");
    } else {
      MQTT.publish(pub8, "1");
    }
    if (status_todos == 1) {
      MQTT.publish(pub0, "1");
    } else {
      MQTT.publish(pub0, "0");
    }
    
 }
       
//Garante funcionamento das conexões WiFi e ao Broker MQTT
    VerificaConexoesWiFIEMQTT();

//Keep-Alive da comunicação com Broker MQTT
    MQTT.loop();
}
