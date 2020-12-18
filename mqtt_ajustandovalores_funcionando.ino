

#include <NTPClient.h>
//#include <WiFi.h> //parece ser a biblioteca pro esp32 mas utilizei a declarada abaixo no esp8266...
#include <ESP8266WiFi.h>// utilizei essa no esp8266
#include <WiFiUdp.h> //funciona tanto no esp32 quanto no esp8266
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient/blob/master/src/PubSubClient.h


const char* ssid = "XXXXXX";                 //SSID da rede WIFI
const char* password =  "XXXXXX";    //senha da rede wifi
//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "broker.mqtt-dashboard.com";   //server
const char* mqttUser = "guilh123";              //user
const char* mqttPassword = "1234t";      //password
const int mqttPort  = 1883;                     //port
const char* mqttTopicSub = "desafio/L1";           //tópico que sera assinado


int ii = 1;

WiFiUDP ntpUDP;
WiFiClient espClient;
PubSubClient client(espClient);

//nao esqueca de ajustar o fuso
const int   utcOffsetInSeconds = -3600 * 3;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);



struct Acionamento {
  String Hora;
  String Minutos;
} aciona[6];

struct Desafio {
  struct Acionamento A[6];
} desafio[3];


void modificaAcionamento(int i, int j, String h_modificada, String m_modificado) {

  desafio[i].A[j].Hora = h_modificada;
  desafio[i].A[j].Minutos = m_modificado;
  leituraAcionamento();

}

void leituraAcionamento(void){

  int i;
  int j;
  for (i = 1; i <= 3; i++)
  {
    for (j = 1; j <= 5; j++)
    {
        Serial.print("Desafio = ");
        Serial.print(i);
        Serial.print(" Acionamento = ");
        Serial.print(j);
        Serial.print("\n");
        Serial.print(desafio[i].A[j].Hora);
        Serial.print(":");
        Serial.print(desafio[i].A[j].Minutos);
        Serial.print("\n");
     }
   }

}

void verificaAcionamento(void) {

  timeClient.update(); //atualiza a hora atual
  String h_atual = String(timeClient.getHours()); //hora atual
  String m_atual = String(timeClient.getMinutes());//minuto atual

  int i; //variável de controle do loop da struct Desafio
  int j; //variável de controle do loop da struct Acionamento

  for (i = 1; i <= 3; i++)
  {
    for (j = 1; j <= 5; j++)
    {
      //se durante o mapeamento houver algum horario que bata com o horario atual
      //é realizado um acionamento durante 1min, no caso o minuto do acionamento.
      if ((desafio[i].A[j].Hora == h_atual) && (desafio[i].A[j].Minutos == m_atual)) {
        Serial.print("Acionamento iniciado pelo desafio nº: ");
        Serial.print(i);
        Serial.print(" e pelo acionamento nº: ");
        Serial.print(j);
        Serial.print("\n");
      }
    }
  }
}

//------Funções do mqtt--------

void reconnect() {
  // Loop até ser reconectado
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT ...");
    if (client.connect(mqttUser)) {
      client.subscribe(mqttTopicSub);
      Serial.println("Conectado");
      Serial.print("Nome do tópico: ");
      Serial.println(mqttTopicSub);
      Serial.println('\n');

    } else {
      Serial.println("tentando novamente em 5 segundos");
      // agurda 5 segundos
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {

  String mensagemRecebida = "\0";
  Serial.print("Mensagem recebida do tópico: ");
  Serial.println(topic);

  Serial.print("Mensagem:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    mensagemRecebida += String((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");
  Serial.println(mensagemRecebida);
  Serial.println("-----------------------");

  //muda os valores conforme msg recebida
  int compare = mensagemRecebida.compareTo("desafio");
  if (compare > 6) { //se a mensagem contiver a palavra 'desafio'+outros_caracteres
    String  horaModificada = String(mensagemRecebida.charAt(21)) + String(mensagemRecebida.charAt(22));
    String  minModificado = String(mensagemRecebida.charAt(24)) + String(mensagemRecebida.charAt(25));
    int indexD = (mensagemRecebida.charAt(7)) - '0';
    int indexA = (mensagemRecebida.charAt(19)) - '0';
    int h1 = ((mensagemRecebida.charAt(21)) - '0') * 10 + ((mensagemRecebida.charAt(22)) - '0');
    int m1 = ((mensagemRecebida.charAt(24)) - '0') * 10 + ((mensagemRecebida.charAt(25)) - '0');

    //o padrão da mensagem deve ser >> desafioXacionamentoY HH:MM
    //sendo x entre 1 e 3, Y entre 1 e 5, HH entre 0 e 23 e MM entre 0 e 59
    if ((indexD >= 1 && indexD <= 3) && (indexA >= 1 && indexA <= 5) && (h1 >= 0 && h1 <= 23) && (m1 >= 0 && m1 <= 59)) {
      //Se os valores de desafio for entre 1 e 3, os de acionamento estiverem entre 1 e 5
      //e os de minuto e segundos estiverem conforme padrão atualiza os valores
      modificaAcionamento(indexD, indexA, horaModificada, minModificado); //chama a função para atualizar os valores se estiverem no padrão
    }
    else {
      //caso o valor enviado não esteja no padrão definido
      Serial.println("Valor inválido");
    }
  }

}

//-----fim funções do mqtt-----------

void setup()
{


  Serial.begin(9600);
  Serial.printf("");
  Serial.printf("");
  //conectando na WiFi
  Serial.printf("Conectando em %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Feito");
  timeClient.begin();


  //primeira definição dos acionamentos, colocar somente quando não houver o txt:
  desafio[2].A[1].Hora = "20";
  desafio[2].A[1].Minutos = "59" ;
  desafio[2].A[2].Hora = "20";
  desafio[2].A[2].Minutos = "30" ;
  desafio[1].A[1].Hora = "20";
  desafio[1].A[1].Minutos = "40" ;
  desafio[1].A[2].Hora = "19";
  desafio[1].A[2].Minutos = "50" ;
  desafio[1].A[5].Hora = "00";
  desafio[1].A[5].Minutos = "50" ;



  //-----MQTT-------
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Conectando ao MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {

      Serial.println("Conectando");

    } else {

      Serial.print("FALHA NA CONEXÃO MQTT");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.subscribe(mqttTopicSub);


}

void loop()
{


  //função que verifica se o horario atual bate com algum dos acionamentos pré determinados
  verificaAcionamento();

  //MQTT
  if (!client.connected())  // Reconecta se perder a conexão
  {
    reconnect();
  }
  client.loop();


}
