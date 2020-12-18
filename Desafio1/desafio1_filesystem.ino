 
#include "FS.h"
#include <ESP8266WiFi.h>
 
const char* ssid = "Net Luciana";                 //SSID da rede WIFI
const char* password =  "41158141";
String buf1;String buf;
 int i = 1;
 
void formatFS(void){
  SPIFFS.format();
}

void createFile(void){
  File wFile;
 
  //Cria o arquivo se ele não existir
  if(SPIFFS.exists("/log.txt")){
    Serial.println("Arquivo ja existe!");
  } else {
    Serial.println("Criando o arquivo...");
    wFile = SPIFFS.open("/log.txt","w+");
 
    //Verifica a criação do arquivo
    if(!wFile){
      Serial.println("Erro ao criar arquivo!");
    } else {
      Serial.println("Arquivo criado com sucesso!");
    }
  }
  wFile.close();
}
 
void deleteFile(void) {
  //Remove o arquivo
  if(!SPIFFS.remove("/log.txt")){
    Serial.println("Erro ao remover arquivo!");
  } else {
    Serial.println("Arquivo removido com sucesso!");
  }
}
 
void writeFile(String msg) {
 
  //Abre o arquivo para adição (append)
  //Inclue sempre a escrita na ultima linha do arquivo
  File rFile = SPIFFS.open("/log.txt","a+");
 
  if(!rFile){
    Serial.println("Erro ao abrir arquivo!");
  } else {
    rFile.println(msg);
    Serial.println(msg);
    
  }
  rFile.close();
}
 
void readFile(void) {
  buf1="\0";
  //Faz a leitura do arquivo
  File rFile = SPIFFS.open("/log.txt","r");
  Serial.println("Reading file...");
  while(rFile.available()) {
    String line = rFile.readStringUntil('\n');
    buf1 += line;
    buf1 += "\n";
  }
  Serial.println(buf1);
  rFile.close();
}


void reescreverArquivo(String novoTexto) {
  buf="\0";
  String a1 = String(novoTexto.charAt(0));
  String a2 = String(novoTexto.charAt(1));
  String a3 = String(novoTexto.charAt(2));
  String a4 = String(novoTexto.charAt(3));
  String a5 = String(novoTexto.charAt(4));
  String a6 = String(novoTexto.charAt(5));
 
  String a21=a1+a2+a3+a4+a5+a6;
  Serial.println(a21);
    
  //Faz a leitura do arquivo
  File rFile = SPIFFS.open("/log.txt","r");
  Serial.println("Reading file...");
  while(rFile.available()) {
    String line = rFile.readStringUntil('\n');
   if(line.startsWith(a21)){
      line= novoTexto;
    }
    buf += line;
    buf += "\n";
  }
  Serial.println(buf);
  deleteFile();
  writeFile(buf);
  readFile();
  rFile.close();
}

 
void closeFS(void){
  SPIFFS.end();
}
 
void openFS(void){
  //Abre o sistema de arquivos
  if(!SPIFFS.begin()){
    Serial.println("Erro ao abrir o sistema de arquivos");
  } else {
    Serial.println("Sistema de arquivos aberto com sucesso!");
  }
}
 
void setup(void){
  //Configura a porta serial para 115200bps
  Serial.begin(9600);
 
  //Abre o sistema de arquivos (mount)
  openFS();


//configura as variáveis
 char* var1 =  "VAR_01 = 6";//estado DF
 String var2 = "VAR_02 = Guilherme";//nome do candidato
 bool VAR_03 [7] = {false, true, false, false, false, false, false};//

//descomentar o próximo comando apenas se usar o delete e queira restaurar os valores padrão
//deleteFile();// deleta o arquivo,caso exista;

  if(!(SPIFFS.exists("/log.txt"))){
   //caso o arquivo nao exista, inserir as variáveis padrões nele; caso exista nao serão recarregados os valores padrões
  Serial.println("teste1");createFile();Serial.println("teste2");//Cria o arquivo caso o mesmo não exista
  writeFile(var1);
  writeFile(var2);
  readFile();
  }
 
 //writeFile("VAR_03 = " +VAR_03);
  //Inicia a conexão WiFi
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    
  }
 
  Serial.println(WiFi.localIP());

}
 
void loop(void){
if (i==1){

  reescreverArquivo("VAR_02 = 31");
//    readFile("teste");
//    Serial.print("\n 1 \n");
//    writeFile("teste");
//    Serial.print("\n 2 \n");
//    readFile("teste");
//    Serial.print("\n 3 \n");
//    Serial.print(buf);
//    if (1){
//      readFile("VAR_02");
//      }
//    Serial.print("\n 4 \n");
//    Serial.println(buf);
//    Serial.print("\n 5 \n");
    i=0;
  }


 
}
