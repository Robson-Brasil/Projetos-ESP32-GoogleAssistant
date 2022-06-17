/**********************************************************************************
//IoT - Automação Residencial
//Dispositivo : ESP32
//MQTT
//Red-Node
//Nora
//Google Assistant
//Autor : Robson Brasil
//Versão : 45
//Última Modificação : 16/06/2022
 **********************************************************************************/

  #include <WiFi.h>
  #include <DHT.h>
  #include <PubSubClient.h>
  #include <WiFiUdp.h>
  
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
//define DHTTYPE DHT11      // DHT 11
#define DHTTYPE DHT22      // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

  DHT dht(DHTPIN, DHTTYPE);

//WiFi Status LED
#define wifiLed 0  //D0

// Update these with values suitable for your network.

const char* ssid = "RVR 2,4GHz";                      //WiFI Name
const char* password = "RodrigoValRobson2021";        //WiFi Password
//const char* mqttServer = "broker.hivemq.com";
const char* mqttServer = "192.168.15.30";             // IP do Broker
const char* mqttUserName = "Robson Brasil";           // MQTT UserName
const char* mqttPwd = "LoboAlfa";                     // MQTT Password
const char* clientID = "ESP32ClientGoogleAssistant";  // Client ID Obs.: Deve ser único

//Tópicos do Subscribe
#define sub1 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor1"   // Ligados ao Nora
#define sub2 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor2"   // Ligados ao Nora
#define sub3 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor3"   // Ligados ao Nora
#define sub4 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor4"   // Ligados ao Nora
#define sub5 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor5"   // Ligados ao Nora
#define sub6 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor6"   // Somente por MQTT
#define sub7 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor7"   // Somente por MQTT
#define sub8 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor8"   // Somente por MQTT
//#define sub9 "ESP32-MinhaCasa/QuartoRobson/Temperatura"
//#define sub10 "ESP32-MinhaCasa/QuartoRobson/Umidade"

//Tópicos do Publish
//#define pub1 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor1"   // Ligados ao Nora
//#define pub2 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor2"   // Ligados ao Nora
//#define pub3 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor3"   // Ligados ao Nora
//#define pub4 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor4"   // Ligados ao Nora
//#define pub5 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor5"   // Ligados ao Nora
//#define pub6 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor6"   // Somente por MQTT
//#define pub7 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor7"   // Somente por MQTT
//#define pub8 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor8"   // Somente por MQTT
#define pub9  "ESP32-MinhaCasa/QuartoRobson/Temperatura"
#define pub10 "ESP32-MinhaCasa/QuartoRobson/Umidade"

  WiFiClient espClient;
  PubSubClient client(espClient);

char str_hum_data[10];
char str_temp_data[10];

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (800)
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
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin1, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin1, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } else if (strstr(topic, sub2)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin2, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin2, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } else if (strstr(topic, sub3)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin3, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin3, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } else if (strstr(topic, sub4)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin4, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin4, LOW);  // Turn the LED off by making the voltage HIGH
    }
  } else if (strstr(topic, sub5)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin5, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin5, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } else if (strstr(topic, sub6)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin6, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin6, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } else if (strstr(topic, sub7)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin7, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin7, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } else if (strstr(topic, sub8)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      data += (char)payload[i];
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin8, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin8, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } else {
    Serial.println("Não Inscrito no Tópico do MQTT");
  }
}

void setup() {
  Serial.begin(115200);
  // Conecta WiFi
  WiFi.begin(ssid, password);
  Serial.println("\nConectando WiFi " + String(ssid));
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
   }   else {
    digitalWrite(wifiLed, LOW);
  }
  
  client.loop();
unsigned long now = millis();
    if (now - lastMsg > 1000) {
    float hum_data = dht.readHumidity();
    Serial.println(hum_data);
    /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
    dtostrf(hum_data, 4, 2, str_hum_data);
    float temp_data = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
    dtostrf(temp_data, 4, 2, str_temp_data);
    lastMsg = now;
    
    Serial.print("Publish MQTT: ");
    Serial.print("Temperatura - "); Serial.print(str_temp_data); Serial.println(F("°C"));
    client.publish("ESP32-MinhaCasa/QuartoRobson/Temperatura", str_temp_data);
    
    Serial.print("Umidade - "); Serial.print(str_hum_data); Serial.println(F("%"));
    client.publish("ESP32-MinhaCasa/QuartoRobson/Umidade", str_hum_data);
        }
}
