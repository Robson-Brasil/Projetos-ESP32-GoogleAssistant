/**********************************************************************************
//IoT - Automação Residencial
//Dispositivo : ESP32
//MQTT
//Red-Node
//Nora
//Google Assistant
//Autor : Robson Brasil
//Versão : 25
//Última Modificação : 25/01/2022
 **********************************************************************************/

  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <PubSubClient.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
  #include <Update.h>
  #include <DHT.h>
  #include <SPIFFS.h>

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

const char* ssid = "RVR 2,4GHz";                //WiFI Name
const char* password = "RodrigoValRobson2021";  //WiFi Password
//const char* mqttServer = "broker.hivemq.com";
const char* mqttServer = "192.168.15.30";             // IP do Broker
const char* mqttUserName = "Robson Brasil";            // MQTT UserName
const char* mqttPwd = "LoboAlfa";                     // MQTT Password
const char* clientID = "ESP32-Client-GoogleAssistant";  // Client ID Obs.: Deve ser único

//Parâmetros de rede
IPAddress local_ip(192, 168, 1, 50);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const uint32_t PORTA = 3232; //A porta que será utilizada (padrão 80)

const char* PARAM_INPUT_1 = "state";

//Algumas informações que podem ser interessantes
const uint32_t chipID = (uint32_t)(ESP.getEfuseMac() >> 32); //um ID exclusivo do Chip...
const String CHIP_ID = "<p> Chip ID do ESP32: " + String(chipID) + "</p>"; // montado para ser usado no HTML
const String VERSION = "<p> Versão OTA: 5.0 </p>"; //Exemplo de um controle de versão

//Informações interessantes agrupadas
const String INFOS = VERSION + CHIP_ID;

//Sinalizador de autorização do OTA
boolean OTA_AUTORIZADO = false;

//Inicia o servidor na porta selecionada
//Aqui testamos na porta 9090, ao invés da 80 padrão
WebServer server(PORTA);

//Páginas HTML utilizadas no procedimento OTA
String verifica = "<!DOCTYPE html><html><head><title>ESP32 WebOTA</title><meta charset='UTF-8'></head><body><h1>ESP32 WebOTA</h1><h2>Digite a chave de verificação.<p>Clique em ok para continuar. . .</p></h2>" + INFOS + "<form method='POST' action='/avalia 'enctype='multipart/form-data'> <p><label>Autorização: </label><input type='text' name='autorizacao'></p><input type='submit' value='Ok'></form></body></html>";
String serverIndex = "<!DOCTYPE html><html><head><title>ESP32 WebOTA</title><meta charset='UTF-8'></head><body><h1>ESP32 WebOTA</h1><h2>Selecione o arquivo para a atualização e clique em atualizar.</h2>" + INFOS + "<form method='POST' action='/update' enctype='multipart/form-data'><p><input type='file' name='update'></p><p><input type='submit' value='Atualizar'></p></form></body></html>";
String Resultado_Ok = "<!DOCTYPE html><html><head><title>ESP32 WebOTA</title><meta charset='UTF-8'></head><body><h1>ESP32 WebOTA</h1><h2>Atualização bem sucedida!</h2>" + INFOS + "</body></html>";
String Resultado_Falha = "<!DOCTYPE html><html><head><title>ESP32 WebOTA</title><meta charset='UTF-8'></head><body><h1>ESP32 WebOTA</h1><h2>Falha durante a atualização. A versão anterior será recarregado.</h2>" + INFOS + "</body></html>";

//Tópicos do Subscribe
#define sub1 "ESP32-MinhaCasa/QuartoRobson/LigarLampada"        // Ligados ao Nora
#define sub2 "ESP32-MinhaCasa/QuartoRobson/LigarBancada"        // Ligados ao Nora
#define sub3 "ESP32-MinhaCasa/QuartoRobson/LigarCooler"         // Ligados ao Nora
#define sub4 "ESP32-MinhaCasa/QuartoRobson/LigarSom"            // Ligados ao Nora
#define sub5 "ESP32-MinhaCasa/QuartoRobson/LigarBluetooth"      // Ligados ao Nora
#define sub6 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor6"   // Somente por MQTT
#define sub7 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor7"   // Somente por MQTT
#define sub8 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor8"   // Somente por MQTT
#define sub9 "ESP32-MinhaCasa/QuartoRobson/Temperatura"
#define sub10 "ESP32-MinhaCasa/QuartoRobson/Umidade"

//Tópicos do Publish
#define pub1 "ESP32-MinhaCasa/QuartoRobson/LigarLampada"        // Ligados ao Nora
#define pub2 "ESP32-MinhaCasa/QuartoRobson/LigarBancada"        // Ligados ao Nora
#define pub3 "ESP32-MinhaCasa/QuartoRobson/LigarCooler"         // Ligados ao Nora
#define pub4 "ESP32-MinhaCasa/QuartoRobson/LigarSom"            // Ligados ao Nora
#define pub5 "ESP32-MinhaCasa/QuartoRobson/LigarBluetooth"      // Ligados ao Nora
#define pub6 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor6"   // Somente por MQTT
#define pub7 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor7"   // Somente por MQTT
#define pub8 "ESP32-MinhaCasa/QuartoRobson/LigarInterruptor8"   // Somente por MQTT
#define pub9 "ESP32-MinhaCasa/QuartoRobson/Temperatura"
#define pub10 "ESP32-MinhaCasa/QuartoRobson/Umidade"

WiFiClient espClient;
PubSubClient client(espClient);

char str_hum_data[10];
char str_temp_data[10];

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (800)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID, mqttUserName, mqttPwd)) {
      Serial.println("MQTT connected");
      // ... and resubscribe
      client.subscribe(sub1);
      client.subscribe(sub2);
      client.subscribe(sub3);
      client.subscribe(sub4);
      client.subscribe(sub5);
      client.subscribe(sub6);
      client.subscribe(sub7);
      client.subscribe(sub8);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if (strstr(topic, sub1)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
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
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(RelayPin8, HIGH);  // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(RelayPin8, LOW);  // Turn the LED off by making the voltage HIGH
    }
    } else {
    Serial.println("unsubscribed topic");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA); //Comfigura o ESP32 como ponto de acesso e estação
  WiFi.begin(ssid, password);// inicia a conexão com o WiFi

  // Inicializa SPIFFS
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Ok");
  } else {
    Serial.println("SPIFFS Falha");
  }

  // Verifica / exibe arquivo
  if (SPIFFS.exists("/Teste.txt")) {
    File f = SPIFFS.open("/Teste.txt", "r");
    if (f) {
      Serial.println("Lendo arquivo:");
      Serial.println(f.readString());
      f.close();
    }
  } else {
    Serial.println("Arquivo não encontrado.");
  }
  
  // Conecta WiFi
  WiFi.begin(ssid, password);
  Serial.println("\nConectando WiFi " + String(ssid));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* Permite definir porta para conexão
      Padrão: ESP8266 - 8266
              ESP32   - 3232              */
  //ArduinoOTA.setPort(port);

  /* Permite definir nome do host
      Padrão: ESP8266 - esp8266-[ChipID]
              ESP32   - esp32-[MAC]       */
   ArduinoOTA.setHostname("ESP32-1");

  /* Permite definir senha para acesso
      Padrão: sem senha                   */
   //ArduinoOTA.setPassword("@Lobo#Alfa@");

  /* Permite definir senha para acesso via Hash MD5
      Padrão: sem senha                   */
  // ArduinoOTA.setPasswordHash("senhaHashMD5");

  // Define funções de callback do processo
  // Início
  ArduinoOTA.onStart([](){
    String s;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      // Atualizar sketch
      s = "Sketch";
    } else { // U_SPIFFS
      // Atualizar SPIFFS
      s = "SPIFFS";
      // SPIFFS deve ser finalizada
      SPIFFS.end();
    }
    Serial.println("Iniciando OTA - " + s);
  });

  // Fim
  ArduinoOTA.onEnd([](){
    Serial.println("\nOTA Concluído.");
  });

  // Progresso
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.print(progress * 100 / total);
    Serial.print(" ");
  });

  // Falha
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.print("Erro " + String(error) + " ");
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Falha de autorização");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Falha de inicialização");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Falha de conexão");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Falha de recebimento");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Falha de finalização");
    } else {
      Serial.println("Falha desconhecida");
    }
  });

  // Inicializa OTA
  ArduinoOTA.begin();

  if (WiFi.status() == WL_CONNECTED) //aguarda a conexão
  {
    //atende uma solicitação para a raiz
    // e devolve a página 'verifica'
    server.on("/", HTTP_GET, []() //atende uma solicitação para a raiz
    {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", verifica);
    });

    //atende uma solicitação para a página avalia
    server.on("/avalia", HTTP_POST, [] ()
    {
      Serial.println("Em server.on /avalia: args= " + String(server.arg("autorizacao"))); //somente para debug

      if (server.arg("autorizacao") != "@Lobo#Alfa@") // confere se o dado de autorização atende a avaliação
      {
        //se não atende, serve a página indicando uma falha
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", Resultado_Falha);
        //ESP.restart();
      }
      else
      {
        //se atende, solicita a página de índice do servidor
        // e sinaliza que o OTA está autorizado
        OTA_AUTORIZADO = true;
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverIndex);
      }
    });

    //serve a página de indice do servidor
    //para seleção do arquivo
    server.on("/serverIndex", HTTP_GET, []()
    {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });

    //tenta iniciar a atualização . . .
    server.on("/update", HTTP_POST, []()
    {
      //verifica se a autorização é false.
      //Se for falsa, serve a página de erro e cancela o processo.
      if (OTA_AUTORIZADO == false)
      {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", Resultado_Falha);
        return;
      }
      //Serve uma página final que depende do resultado da atualização
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", (Update.hasError()) ? Resultado_Falha : Resultado_Ok);
      delay(1000);
      ESP.restart();
    }, []()
    {
      //Mas estiver autorizado, inicia a atualização
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START)
      {
        Serial.setDebugOutput(true);
        Serial.printf("Atualizando: %s\n", upload.filename.c_str());
        if (!Update.begin())
        {
          //se a atualização não iniciar, envia para serial mensagem de erro.
          Update.printError(Serial);
        }
      }
      else if (upload.status == UPLOAD_FILE_WRITE)
      {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
          //se não conseguiu escrever o arquivo, envia erro para serial
          Update.printError(Serial);
        }
      }
      else if (upload.status == UPLOAD_FILE_END)
      {
        if (Update.end(true))
        {
          //se finalizou a atualização, envia mensagem para a serial informando
          Serial.printf("Atualização bem sucedida! %u\nReiniciando...\n", upload.totalSize);
        }
        else
        {
          //se não finalizou a atualização, envia o erro para a serial.
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      else
      {
        //se não conseguiu identificar a falha no processo, envia uma mensagem para a serial
        Serial.printf("Atualização falhou inesperadamente! (possivelmente a conexão foi perdida.): status=%d\n", upload.status);
      }
    });

    server.begin(); //inicia o servidor

    Serial.println(INFOS); //envia as informações armazenadas em INFOS, para debug

    //Envia ara a serial o IP atual do ESP
    Serial.print("Servidor em: ");
    Serial.println( WiFi.localIP().toString() + ":" + PORTA);

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();  
  
  }
  else
  {
    //avisa se não onseguir conectar no WiFi
    Serial.println("Falha ao conectar ao WiFi.");
  }
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

  setup_wifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void loop() {
    //Manipula Clientes Conectados
    server.handleClient();
    ArduinoOTA.handle();
  if (!client.connected()) {
    digitalWrite(wifiLed, HIGH);
    reconnect();
  } else {
    digitalWrite(wifiLed, LOW);
  }
  client.loop();
unsigned long now = millis();
    if (now - lastMsg > 2000) {
    float hum_data = dht.readHumidity();
    Serial.println(hum_data);
    /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
    dtostrf(hum_data, 4, 2, str_hum_data);
    float temp_data = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
    dtostrf(temp_data, 4, 2, str_temp_data);
    lastMsg = now;
    Serial.print("Publish message: ");
    Serial.print("Temperatura - "); Serial.println(str_temp_data);
    client.publish("ESP32-MinhaCasa/QuartoRobson/Temperatura", str_temp_data);
    Serial.print("Umidade - "); Serial.println(str_hum_data);
    client.publish("ESP32-MinhaCasa/QuartoRobson/Umidade", str_hum_data);
  }
 }
