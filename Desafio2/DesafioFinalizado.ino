//    Padrão de preenchimento :     desafioXacionamentoY hh:mm
//         no envio                 2>= X >=1       5>= Y>=1  
//     da mensagem pelo MQTT         X número do desafio
//                                   Y número do acionamento
// IMPORTANTE: caso queira resetar e limpar o txt ->  descomentar linha423

#include <NTPClient.h>      //https://github.com/taranais/NTPClient/blob/master/NTPClient.h
#include <WiFi.h>         //parece ser a biblioteca pro esp32 mas utilizei a declarada abaixo no esp8266...
//#include <ESP8266WiFi.h>// utilizei essa no esp8266
#include <WiFiUdp.h>       //funciona tanto no esp32 quanto no esp8266
#include <PubSubClient.h>  //https://github.com/knolleary/pubsubclient/blob/master/src/PubSubClient.h
#include "FS.h"            //biblioteca pra utilizar SPIFS 

const char* ssid = "XXXXX";       //SSID da rede WIFI >> Var3
const char* password =  "XXXX";    //senha da rede wifi >> Var4
//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "broker.mqtt-dashboard.com";   //server
const char* mqttUser = "guilh123";       //user
const char* mqttPassword = "1234t";      //password
const int mqttPort  = 1883;              //port
const char* mqttTopicSub = "desafio/L1"; //tópico que sera assinado

String buf1; String buf; String buf2;
int COUNT_TESTE = 1;
volatile int STATE = 1;

//ajusta o fuso
const int   utcOffsetInSeconds = -3600 * 3;

WiFiUDP ntpUDP;
WiFiClient espClient;
PubSubClient client(espClient);
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); //NTP PARA ACESSAR O HORÁRIO


// -------Estrutura que guarda os acionamentos pra cada desafio----------
struct Acionamento {
  String Hora;
  String Minutos;
} aciona[6];

struct Desafio {
  struct Acionamento A[6];
} desafio[2];

//---------Função que modifica os valores caso receba no padrão---------
void modificaAcionamento(int i, int j, String h_modificada, String m_modificado) {

  desafio[i].A[j].Hora = h_modificada;
  desafio[i].A[j].Minutos = m_modificado;
  String reescreveAcionamento = "\0" ;
  reescreveAcionamento = "desafio";
  reescreveAcionamento += String(i);
  reescreveAcionamento += "acionamento";
  reescreveAcionamento += String(j);
  reescreveAcionamento += " ";
  reescreveAcionamento += h_modificada;
  reescreveAcionamento += ":";
  reescreveAcionamento += m_modificado;
  reescreverArquivo2(reescreveAcionamento);
  // informa, ao final do txt, a última mudança no arquivo
  String informaLog = "\0";
  String h_atual2 = String(timeClient.getHours()); //hora atual
  String m_atual2 = String(timeClient.getMinutes());//minuto atual
  informaLog = "Log:";
  informaLog += " Foi modificado o desafio ";
  informaLog += String((i));
  informaLog += " e o acionamento ";
  informaLog += String(j);
  informaLog += " para ";
  informaLog += String(desafio[i].A[j].Hora);
  informaLog += ":";
  informaLog += String(desafio[i].A[j].Minutos);
  informaLog += ". Modificado em: ";
  informaLog +=  String(h_atual2);
  informaLog += ":";
  informaLog += String(m_atual2);
  reescreverArquivo(informaLog);

}


//-------Faz a leitura do txt e atualiza os valores Structs Desafio e Acionamento----------
void leituraAcionamento(void) {
  String buf1 = "\0";
  int i;
  int j;
  //Faz a leitura do arquivo
  File rFile = SPIFFS.open("/log.txt", "r");
  Serial.println("Reading file...");
  while (rFile.available()) {
    String line = rFile.readStringUntil('\n');
    for (i = 1; i <= 2; i++)
    {
      for (j = 1; j <= 5; j++)
      {

        String a21 = "\0";
        a21 = "desafio" + String(i) + "acionamento" + String(j);

        if (line.startsWith(a21)) {
          desafio[i].A[j].Hora =  String(line.charAt(21)) + String(line.charAt(22));
          desafio[i].A[j].Minutos =  String(line.charAt(24)) + String(line.charAt(25));
        }
      }
      buf1 = String(line.charAt(21)) + String(line.charAt(22)) + ":" + String(line.charAt(24)) + String(line.charAt(25));
    }
  }
  Serial.println(buf1);
  rFile.close();
}

//-------Faz a ESCRITA das Structs Desafio e Acionamento ----------
void escritaAcionamento(void) {
  String buf2 = "\0";
  int CNT1;
  int CNT2;
  for (CNT1 = 1; CNT1 <= 2; CNT1++)
  {
    for (CNT2 = 1; CNT2 <= 5; CNT2++)
    {
      buf2 += "desafio";
      buf2 += String((CNT1));
      buf2 += "acionamento";
      buf2 += String(CNT2);
      buf2 += " ";
      buf2 += String(desafio[CNT1].A[CNT2].Hora);
      buf2 += ":";
      buf2 += String(desafio[CNT1].A[CNT2].Minutos);
      buf2 += ("\n");
    }
  }

  writeFile(buf2);

}



//-------Verifica se o acionamento vai ocorrer comparando os valores salvos com o hora´rio atual-----------
void verificaAcionamento(void) {

  timeClient.update(); //atualiza a hora atual
  String h_atual = String(timeClient.getHours()); //hora atual
  String m_atual = String(timeClient.getMinutes());//minuto atual
  int h2 = (h_atual.charAt(1) - '0');
  String h1=h_atual;
  if(h2<0 || h2>9){ //se o ultimo digito da hora nao estiver entre 0 e 9 significa que a hora esta apenas com um digito
    h1= "0"+String(h_atual.charAt(0));  
  }  
  int m2 = (m_atual.charAt(1) - '0');
  String m1=m_atual;
  if(m2<0 || m2>9){ //se o ultimo digito do min não estiver entre 0 e 9 significa que o min esta apenas com um digito
    m1= "0"+String(m_atual.charAt(0));  
  }
  int i; //variável de controle do loop da struct Desafio
  int j; //variável de controle do loop da struct Acionamento
  
  for (i = 1; i <= 2; i++)
  {
    for (j = 1; j <= 5; j++)
    {
      //se durante o mapeamento houver algum horario que bata com o horario atual
      //é realizado um acionamento durante 1min, no caso o minuto do acionamento.
      if ((desafio[i].A[j].Hora == h1) && (desafio[i].A[j].Minutos == m1)) {
        Serial.print("Acionamento iniciado pelo desafio nº: ");
        Serial.print(i);
        Serial.print(" e pelo acionamento nº: ");
        Serial.print(j);
        Serial.print("\n");
      }
    }
  }
}


//---------FUNÇÕES PRA USAR O SPIFFS----------
void formatFS(void) {
  SPIFFS.format();
}

void createFile(void) {
  File wFile;

  //Cria o arquivo se ele não existir
  if (SPIFFS.exists("/log.txt")) {
    Serial.println("Arquivo ja existe!");
  } else {
    Serial.println("Criando o arquivo...");
    wFile = SPIFFS.open("/log.txt", "w+");

    //Verifica a criação do arquivo
    if (!wFile) {
      Serial.println("Erro ao criar arquivo!");
    } else {
      Serial.println("Arquivo criado com sucesso!");
    }
  }
  wFile.close();
}

void deleteFile(void) {
  //Remove o arquivo
  if (!SPIFFS.remove("/log.txt")) {
    Serial.println("Erro ao remover arquivo!");
  } else {
    Serial.println("Arquivo removido com sucesso!");
  }
}

void writeFile(String msg) {

  //Abre o arquivo para adição (append)
  //Inclue sempre a escrita na ultima linha do arquivo
  File rFile = SPIFFS.open("/log.txt", "a+");

  if (!rFile) {
    Serial.println("Erro ao abrir arquivo!");
  } else {
    rFile.println(msg);
    //Serial.println(msg);
  }
  rFile.close();
}

void readFile(void) {
  buf1 = "\0";
  //Faz a leitura do arquivo
  File rFile = SPIFFS.open("/log.txt", "r");
  Serial.println("Reading file...");
  while (rFile.available()) {
    String line = rFile.readStringUntil('\n');
    buf1 += line;
    buf1 += "\n";
  }
  Serial.println(buf1);
  rFile.close();
}

//FUNÇÃO QUE VAI LIMPAR O ARQUIVO E ESCREVER COM OS VALORES NOVOS
void reescreverArquivo(String novoTexto) {
  buf = "\0";
  String a1 = String(novoTexto.charAt(0));
  String a2 = String(novoTexto.charAt(1));
  String a3 = String(novoTexto.charAt(2));
  String a4 = String(novoTexto.charAt(3));
  String a5 = String(novoTexto.charAt(4));
  String a6 = String(novoTexto.charAt(5));

  String a21 = a1 + a2 + a3 + a4 + a5 + a6;
  //Serial.println(a21);

  //Faz a leitura do arquivo
  File rFile = SPIFFS.open("/log.txt", "r");
  Serial.println("Reading file...");
  while (rFile.available()) {
    String line = rFile.readStringUntil('\n');
    if (line.startsWith(a21)) {
      line = novoTexto;
    }
    buf += line;
    buf += "\n";
  }
  // Serial.println(buf);
  deleteFile();
  writeFile(buf);
  readFile();
  rFile.close();
}

//FUNÇÃO QUE VAI REESCREVER A PARTE DO ACIONAMENTO
void reescreverArquivo2(String novoTexto) {
  buf = "\0";
  String a1 = "desafio";
  String a2 = String(novoTexto.charAt(7));
  String a3 = "acionamento";
  String a4 = String(novoTexto.charAt(19));

  String a21 = a1 + a2 + a3 + a4;
  Serial.println(a21);

  //Faz a leitura do arquivo
  File rFile = SPIFFS.open("/log.txt", "r");
  Serial.println("Reading file...");
  while (rFile.available()) {
    String line = rFile.readStringUntil('\n');
    if (line.startsWith(a21)) {
      line = novoTexto;
    }
    buf += line;
    buf += "\n";
  }
  //Serial.println(buf);
  deleteFile();
  writeFile(buf);
  readFile();
  rFile.close();
}

void closeFS(void) {
  SPIFFS.end();
}

void openFS(void) {
  //Abre o sistema de arquivos
  if (!SPIFFS.begin()) {
    Serial.println("Erro ao abrir o sistema de arquivos");
  } else {
    Serial.println("Sistema de arquivos aberto com sucesso!");
  }
}
//------FIM DAS FUNÇÕES ENVOLVENDO O SPIFFS-----------

//------FUNÇÕES MQTT------------------
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
    if ((indexD >= 1 && indexD <= 2) && (indexA >= 1 && indexA <= 5) && (h1 >= 0 && h1 <= 23) && (m1 >= 0 && m1 <= 59)) {
      //Se os valores de desafio for entre 1 e 2, os de acionamento estiverem entre 1 e 5
      //e os de minuto e segundos estiverem conforme padrão atualiza os valores
      modificaAcionamento(indexD, indexA, horaModificada, minModificado); //chama a função para atualizar os valores se estiverem no padrão
    }
    else {
      //caso o valor enviado não esteja no padrão definido
      Serial.println("Valor inválido");
    }
  }

}

//-----FIM FUNÇÕES MQTT----------

//INICIALIZAÇÃO
void setup() {

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
  //--------FIM DEFINIÇÃO MQTT---------------

  //----------SPIFSS----------
  openFS();//Abre o sistema de arquivos (mount)
  //configura as variáveis
  char* var1 =  "VAR_01 = 6";//estado DF
  String var2 = "VAR_02 = Guilherme";//nome do candidato
  bool var3 [7] = {false, false, false, false, false, true, false};//(0) domingo
  
  //descomentar o próximo comando apenas se usar o delete e queira restaurar os valores padrão
  //deleteFile();// deleta o arquivo,caso exista;
  if (!(SPIFFS.exists("/log.txt"))) {
    //caso o arquivo nao exista, inserir as variáveis padrões nele; caso exista nao serão recarregados os valores padrões
    createFile();//Cria o arquivo caso,já que não existe, na primeira vez
    writeFile(var1);
    writeFile(var2);
    String var33= "VAR_03 = "+String(var3[0])+String(var3[1])+String(var3[2])+String(var3[3])+String(var3[4])+String(var3[5])+String(var3[6]);
    writeFile(var33);
    String var4="VAR_04 = "+String(ssid);
    writeFile(var4);
    String var5="VAR_05 = "+String(password);
    writeFile(var5);
   
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
    STATE = 0;
  }
  //--------END SPIFFS--------

}

void loop()
{


  //função que verifica se o horario atual bate com algum dos acionamentos pré determinados
  verificaAcionamento();

//realiza uma vez --- caso seja a primeira inicialização, sem arquivos preenche o padrão
//caso State=1 já nao é a primeira inicialização e toda vez q reseta são lidos os valores do txt e repassados pras structs
  if (COUNT_TESTE == 1) {
    if (STATE == 0) {
      //escreve no txt os valores das structs na primeira escrita
      escritaAcionamento();
      writeFile("Log: Foi criado o arquivo - ainda sem mudanças");
      readFile();
      STATE = 1;
    }
    else {
      //escrever na struct os valores salvos no txt;
      void leituraAcionamento();
    }
  }

  //MQTT
  if (!client.connected())  // Reconecta se perder a conexão
  {
    reconnect();
  }
  client.loop();

}
