#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "./DNSServer.h"
#include <ESP8266WebServer.h>
#include "FS.h"
#include <stdlib.h>

//#define ENCRYPT
//#define CRC

#ifdef ENCRYPT

  #include <pgmspace.h>
  #include <Crypto.h>
  #include <AES.h>
  #include <GCM.h>

#endif

//lora
#include <SPI.h>              // include libraries
#include <LoRa.h>
const int csPin = 15;          // LoRa radio chip select
const int resetPin = 16;       // LoRa radio reset
const int irqPin = 5;         // change for your board; must be a hardware interrupt pin
//byte msgCount = 0;            // count of outgoing messages
//int interval = 2000;          // interval between sends
//long lastSendTime = 0;        // time of last packet send


//esp
IPAddress         apIP(10, 10, 10, 1);
DNSServer         dnsServer;
ESP8266WebServer  webServer(80);

const byte        DNS_PORT = 53;
const String      messagesFile = "/messages.txt";
const String      settingsfile = "/settings.cfg";
const String      jsfile = "/javascript.js";
const String      cssfile = "/style.css";
const String      chatFile = "/chat.html";
const char*       wifiName = "Chat With Me1";
#define           wifipass "123456789"
String            chatHtml;
String filerecieved;
String settings;

const int fileBuffersize = 1000;

File fsUploadFile;

int PA = 0;
int TxPower = 14;
long SignalBandwidth = 125E3;
int SpreadingFactor = 7;
int CodingRate4 = 5;
int PreambleLength = 8;
int SyncWord = 0xF3;
int Gain = 0;
volatile int incomingLoraPacketSize = 0;
volatile bool doLoraRead = false;

#ifdef ENCRYPT

  struct TestVector
  {
      const char *name;
      byte key[32];
      byte plaintext[16];
      byte ciphertext[16];
  };

    // Define the ECB test vectors from the FIPS specification.
  static TestVector const testVectorAES128 = {
      .name        = "AES-128-ECB",
      .key         = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
      .plaintext   = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
      .ciphertext  = {0x69, 0xC4, 0xE0, 0xD8, 0x6A, 0x7B, 0x04, 0x30,
                      0xD8, 0xCD, 0xB7, 0x80, 0x70, 0xB4, 0xC5, 0x5A}
  };
  static TestVector const testVectorAES192 = {
      .name        = "AES-192-ECB",
      .key         = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},
      .plaintext   = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
      .ciphertext  = {0xDD, 0xA9, 0x7C, 0xA4, 0x86, 0x4C, 0xDF, 0xE0,
                      0x6E, 0xAF, 0x70, 0xA0, 0xEC, 0x0D, 0x71, 0x91}
  };
  static TestVector const testVectorAES256 = {
      .name        = "AES-256-ECB",
      .key         = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
      .plaintext   = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
      .ciphertext  = {0x8E, 0xA2, 0xB7, 0xCA, 0x51, 0x67, 0x45, 0xBF,
                      0xEA, 0xFC, 0x49, 0x90, 0x4B, 0x49, 0x60, 0x89}
  };

  AES128 aes128;
  AES192 aes192;
  AES256 aes256;
  byte buffer[16];

#endif

void setup() {
  Serial.begin(9600);
  SPIFFS.begin();
  //WiFi.begin();

  //lora setup
  Serial.println("LoRa Duplex - Set sync word");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(431E6)) {             // initialize radio at 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  settings = messageRead(settingsfile);
  String tempraryString = "";
  int settingindex = 0;

  for (int i = 0; i < settings.length() ; i++)
  {
    if (settings.charAt(i) != ' ')
    {
      tempraryString += settings.charAt(i);
    }
    else
    {
      switch (settingindex)
      {
      case 0:
        PA = tempraryString.toInt();
      case 1:
        TxPower = tempraryString.toInt();
        break;
      case 2:
        SignalBandwidth = tempraryString.toFloat();
        break;
      case 3:
        SpreadingFactor = tempraryString.toInt();
        break;
      case 4:
        CodingRate4 = tempraryString.toInt();
        break;
      case 5:
        PreambleLength = tempraryString.toInt();
        break;
      case 6:
        SyncWord = strtol(tempraryString.c_str(),NULL,0);
        break;
      case 7:
        Gain = tempraryString.toInt();
        break;
      default:
        break;
      }
      settingindex++;
      tempraryString = "";
    }
    
  }
  
  if(PA == 0){
    LoRa.setTxPower(TxPower, PA_OUTPUT_PA_BOOST_PIN);                  // Supported values are 2 to 20 for PA_OUTPUT_PA_BOOST_PIN, and 0 to 14 for PA_OUTPUT_RFO_PIN. default: 17
    Serial.printf("TxPowerMode: PA_OUTPUT_PA_BOOST_PIN\n");
  }else if(PA == 1){
    LoRa.setTxPower(TxPower, PA_OUTPUT_RFO_PIN);
    Serial.printf("TxPowerMode: PA_OUTPUT_RFO_PIN\n");
  }else{
    Serial.printf("TxPower Error\n");
  }
  Serial.printf("TxPower: ");
  Serial.println(TxPower);

  LoRa.setSignalBandwidth(SignalBandwidth);  // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3 default: 125E3  62.5
  Serial.printf("SignalBandwidth: ");
  Serial.println(SignalBandwidth);

  LoRa.setSpreadingFactor(SpreadingFactor);  // 7-12 default: 7      12
  Serial.printf("SpreadingFactor: ");
  Serial.println(SpreadingFactor);

  LoRa.setCodingRate4(CodingRate4);          // Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8. The coding rate numerator is fixed at 4. default: 5     8
  Serial.printf("CodingRate4: ");
  Serial.println(CodingRate4);

  LoRa.setPreambleLength(PreambleLength);   // Supported values are between 6 and 65535. default: 8
  Serial.printf("PreambleLength: ");
  Serial.println(PreambleLength);

  LoRa.setSyncWord(SyncWord);               // ranges from 0x00-0xFF, default 0x34, see API docs
  Serial.printf("SyncWord: ");
  Serial.println(SyncWord);

  #ifdef CRC
    LoRa.enableCrc();                        // disabled by default
    Serial.printf("CRC Enabled\n");
  #endif

  LoRa.setGain(Gain);                      // Supported values are between 0 and 6. If gain is 0, AGC will be enabled and LNA gain will not be used. Else if gain is from 1 to 6, AGC will be disabled and LNA gain will be used. default: 0
  Serial.printf("Gain: ");
  Serial.println(Gain);


  Serial.printf("LoRa init succeeded.\n");
  LoRa.onReceive(onLoraReceive);
  LoRa.receive();

  //setting up AP
  WiFi.mode(WIFI_AP);              
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(wifiName, wifipass);
  dnsServer.start(DNS_PORT, "*", apIP);
  chatHtml = messageRead(chatFile);
  webServer.begin();

  setupAppHandlers();
  handleSendMessage();
  showChatPage();

}

#ifdef ENCRYPT

  String perfCipherEncrypt(BlockCipher *cipher, const struct TestVector *test, String message)
  {
      int count;

      int len = message.length();

      while (len%16 != 0)
      {
        len++;
      }

      char messageArray[len];


      for (count = 0; count <= message.length(); count++){
        messageArray[count] = message.charAt(count);
      }
      crypto_feed_watchdog();
      cipher->setKey(test->key, cipher->keySize());

      Serial.print(test->name);
      Serial.print(" Encrypt ... ");
      for (count = 0; count < len/16; count++){
        for (int i = 0; i < 16; i++)
        {
          buffer[i] = (byte)messageArray[i*(count+1)];
        }
        crypto_feed_watchdog();
        cipher->encryptBlock(buffer, buffer);
        for (int i = 0; i < 16; i++)
        {
          messageArray[i*(count+1)] = (char)buffer[i];
        }
      }
      message = "";
      for (count = 0; count <= len; count++){
        message += messageArray[count];
      }
      cipher->clear();
      Serial.println(message);
      return message;
  }

  String perfCipherDecrypt(BlockCipher *cipher, const struct TestVector *test, String message)
  {

      int count;

      int len = message.length();
      len--;
      if (len%16 != 0)
      {
        return "Error message length: "+ String(len);
      }

      char messageArray[len];

      for (count = 0; count <= len; count++){
        messageArray[count] = message.charAt(count);
      }
      crypto_feed_watchdog();
      cipher->setKey(test->key, cipher->keySize());

      Serial.print(test->name);
      Serial.print(" Decrypt ... ");
      for (count = 0; count < len/16; count++){
        for (int i = 0; i < 16; i++)
        {
          buffer[i] = (byte)messageArray[i*(count+1)];
        }
        crypto_feed_watchdog();
        cipher->decryptBlock(buffer, buffer);
        for (int i = 0; i < 16; i++)
        {
          messageArray[i*(count+1)] = (char)buffer[i];
        }
      }
      message = "";
      for (count = 0; count <= len; count++){
        message += messageArray[count];
      }

      cipher->clear();
      Serial.println(message);
      return message;
  }

#endif

void sendLoraMessage(String outgoing) {

  #ifdef ENCRYPT
    outgoing = perfCipherEncrypt(&aes256, &testVectorAES256, outgoing);
  #endif

  LoRa.beginPacket(false);                   // start explicit packet
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it

}

void handleSendMessage() {
  if (webServer.hasArg("message")) {
    String message = webServer.arg("message");
    messageWrite(messagesFile, message + "\n" , "a+");
    
    
    webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "text/plain", "Message Sent");
    
    
    //lora send message
    sendLoraMessage(message);
    Serial.println("Sending " + message);
    
    LoRa.receive();

  }
}

void handleClearMessages() {
  SPIFFS.remove(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", "File Deleted");
}

void showChatPage() {
  webServer.send(200, "text/html", chatHtml);
}

void showMessages() {
  String messages = messageRead(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", messages);
}

void handleSettingstoesp() 
{
  if (webServer.hasArg("settingsToEsp")) {
    String setting = webServer.arg("settingsToEsp");
    Serial.println(setting);
    messageWrite(settingsfile, setting, "w");
    
    webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "text/plain", "Setting stored");
    ESP.restart();
  }
}

void settingsToBrowser()
{
  String tempSetting; 
  tempSetting = messageRead(settingsfile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", tempSetting);
}

void setupAppHandlers() {

  webServer.on("/", showChatPage);
  webServer.on("/sendMessage", handleSendMessage);
  webServer.on("/readMessages", showMessages);
  webServer.on("/clearMessages", handleClearMessages);
  webServer.on("/upload", HTTP_POST, [](){ webServer.send(200); }, handleFileUpload);
  webServer.on("/settingsToesp", handleSettingstoesp);
  webServer.on("/settingsToBrowser", settingsToBrowser);
  webServer.onNotFound([]() {
    if (!handleFileRead(webServer.uri()))                  // send it if it exists
      webServer.send(404, "text/plain", "404: Not Found");
  });
}

String messageRead(String name) {
  String contents;
  int i;
  File file = SPIFFS.open(name, "a+");
  for (i = 0; i < file.size(); i++)
  {
    contents += (char)file.read();
  }
  file.close();
  return contents;
}

void messageWrite(String name, String content, String mode) {
  File file = SPIFFS.open(name.c_str(), mode.c_str());
  file.write((uint8_t *)content.c_str(), content.length());
  file.close();
}

void readLoraMessage(){
  if (doLoraRead) {
    String message = "";
    Serial.print("Received packet '");
    
    // read packet

    for (int i = 0; i < incomingLoraPacketSize; i++) {
      message += (char)LoRa.read();
    }
    #ifdef ENCRYPT

      message = perfCipherDecrypt(&aes256, &testVectorAES256, message);
    
    #endif

    Serial.printf("packet size: ");
    Serial.println(incomingLoraPacketSize);

    Serial.println(message);
    if(message.endsWith("ping") || message.endsWith("Ping")){
      sendLoraMessage("Pong");
    }
    else if(message.startsWith("/files/")){
      filerecieved = message;
    }
    else if(message.startsWith("F;") && message.endsWith(";E")){
      message.remove(0,2);
      message.remove(message.length()-2,2);

      byte tmpbyte[message.length()+1];
      message.getBytes(tmpbyte, message.length()+1);

      for(int z = 0; z <= message.length() ; z++){
        lorafilewrite(filerecieved, tmpbyte[z]);
        Serial.print(tmpbyte[z]);
      }
      
    }
    else if(message.endsWith("TheEnd;")){
      messageWrite(messagesFile, "<a href=\"" + filerecieved + "\">File uploaded: " + filerecieved + "</a>", "a+");
    }
    

    messageWrite(messagesFile, message + "\n" , "a+");
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    doLoraRead = false;
  }
}

void onLoraReceive(int packetSize) {
  // received a packet
  doLoraRead = true;
  incomingLoraPacketSize = packetSize;
}

void lorafilewrite(String address, byte data){
  File X20 = SPIFFS.open(address, "a+");
  if (!X20) {
    Serial.println("Error opening file X20 for writing");
    return;
  }
  else{
    int j = 0;
    X20.print(data);
  }
  X20.close();
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = webServer.upload();
  
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    filename.replace(" ", "_");
    if(!filename.startsWith("/files/")) filename = "/files/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      String filename = upload.filename;
      filename.replace(" ", "_");
      if(!filename.startsWith("/files/")) filename = "/files/"+filename;

      messageWrite(messagesFile, "<a href=\"" + filename + "\">File uploaded: " + filename + "</a>", "a+");
      
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);

      //server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      webServer.send(303);

      //sending file to lora
      File loraFile = SPIFFS.open(filename, "r+");
      if (!loraFile) {
        Serial.println("Failed to open lorafile for reading");
      }
      sendLoraMessage(filename);
      while (loraFile.available()) {
        String LF = "";
        LF = String(loraFile.read());
        sendLoraMessage("F;"+ LF +";E");
        Serial.print(LF);
      }
      loraFile.close();
      sendLoraMessage("TheEnd;");

    } else {
      webServer.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path = pathWithGz;                                      // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = webServer.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void loop() {
  readLoraMessage();
  dnsServer.processNextRequest();
  webServer.handleClient();
}
