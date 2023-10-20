
void Read64byte(char *Host);
#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif
#include <FS.h>
#include <LittleFS.h>
#include <time.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


/*************************************************************/
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[50];
int value = 0;
/*************************************************************/
// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the SD card interfaces setting and mounting
#include <addons/SDHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Hack network"
#define WIFI_PASSWORD "fci.2020"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDE6nizC6oeE63qyz1W-1K2na5MV-omnSM"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "mahmoudmenim9@gmail.com"
#define USER_PASSWORD "123456"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "fota-73dcc.appspot.com"
const char* mqtt_server = "test.mosquitto.org";// MQTT broker

/* OLED display width, in pixels */
#define SCREEN_WIDTH 128 
/* OLED display height, in pixels */
#define SCREEN_HEIGHT 64 

/* Oled Display Functions */
void DisplayFlashed();
void DisplayLoading();
void DisplayIsUploading();

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif





// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



long timezone = 0;
byte daysavetime = 1;

void listDir(const char *dirname) {
  Serial.printf("Listing directory: %s\n", dirname);

  Dir root = LittleFS.openDir(dirname);

  while (root.next()) {
    File file = root.openFile("r");
    Serial.print("  FILE: ");
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm *tmstruct = localtime(&cr);
    Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
}

void readFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) { Serial.write(file.read()); }
  file.close();
}




void setup()
{

    Serial.begin(115200);
    Serial1.begin(115200);
    pinMode(13,OUTPUT);
 char hexString[] = "ff";

  // Convert the hexadecimal string to a one-byte number
  char byteValue = strtol(hexString, NULL, 16);
  Serial.printf("====================%d==========\n",byteValue);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    /* Assign download buffer size in byte */
    // Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
    // The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
    config.fcs.download_buffer_size = 2048;

    Firebase.begin(&config, &auth);

    // if use SD card, mount it.
    SD_Card_Mounting(); // See src/addons/SDHelper.h



    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

/*************************************************************/
int x=0,y=0;
char buf[2];

char paket[74];
void Read64byte(char *Host);
void callback(char* topic, byte* payload, unsigned int length) {
  String string;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
     string+=((char)payload[i]);  
  }
    Serial.print(string);
if (topic ="/mahmoud/servo")  /// esp32 subscribe topic //0,50,100   
    Serial.print(" ");
   int status = string.toInt();   
   Serial.println(status);
   if(status == 0)
   {        if(x==1){
            DisplayIsUploading();
            DisplayLoading(6 , 2);
            Read64byte(paket);
            DisplayLoading(15 , 38);
            DisplayFlashed();
            Serial1.write("Bootloader End..");
            
            }
            x++;
   }
   else if(status==50)
   {

      Serial1.write("Bootloader Ereas");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(3, 16);
      // Display static text
     display.println("The Code Has been   Ereased Successfully!");
     display.display();
      
   }

    delay(15);  
 }
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESPClient")) {
      Serial.println("connected");
      client.subscribe("/mahmoud/servo");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds"); 
      delay(5000);
   }}}
   /*************************************************************/
// The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
    if (info.status == firebase_fcs_download_status_init)
    {
        Serial.printf("Downloading file %s (%d) to %s\n", info.remoteFileName.c_str(), info.fileSize, info.localFileName.c_str());
    }
    else if (info.status == firebase_fcs_download_status_download)
    {
        Serial.printf("Downloaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    }
    else if (info.status == firebase_fcs_download_status_complete)
    {
        Serial.println("Download completed\n");
    }
    else if (info.status == firebase_fcs_download_status_error)
    {
        Serial.printf("Download failed, %s\n", info.errorMsg.c_str());
    }

      /*Buzzer*/
    pinMode(12,OUTPUT);
    pinMode(13,OUTPUT);
  
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    /* Oled Error */
    for(;;);
  }

}


void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.println("\nDownload file...\n");

        // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
        if (!Firebase.Storage.download(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, "TestBootloader.bin" /* path of remote file stored in the bucket */, "/updat.bin" /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, fcsDownloadCallback /* callback function */))
            Serial.println(fbdo.errorReason());
           
    }
          paket[0]=74;
          paket[1]=0x16;
          paket[2]=0x08;
          paket[3]=0x00;
          paket[4]=0x80;
          paket[5]=0x00;
          paket[6]=64;          
       /*   if(x==0)
          {

            delay(5000);
            
            DisplayIsUploading();
            DisplayLoading(6 , 2);
            Read64byte(paket);
            DisplayLoading(15 , 38);
            DisplayFlashed();

            x=1;
            Serial1.write("Bootloader End..");
            
          }*/
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void Read64byte(char *Host)
{
    int i=0;
    char buf[2]={0};
    char buff[2]={0};
    File file = LittleFS.open("/updat.bin", "r");
    i=0;
    while(file.available())
    {
        delay(100);
        Serial1.write("Bootloader Start");
        i=0;
        
        while (i<64)
        {
          size_t len = file.readBytes(buf,2);
          char bte = strtol(buf, NULL, 16); 
          Host[7+i]=bte;
          Serial.printf("host[%d]=",7+i);
          Serial.printf("%x\n",Host[7+i]);
          i++;
        }
        i=0;
        *(int*)(&paket[2])=0x8008000+i*64;
        Serial1.write(paket[0]);
        Serial1.write(paket[1]);
        Serial1.write(paket[2]);
        Serial1.write(paket[3]);
        Serial1.write(paket[4]);
        Serial1.write(paket[5]);
        Serial1.write(paket[6]);
        while (i<64)
        {
          Serial1.write(Host[7+i]);
          i++;
        }
        while(!Serial.available())
        {
                  
        }
        Serial.printf("bootloader answer is %d",Serial.read());
        Serial.read();
        i=0;
        }

    file.close();
}





/*---- Oled Display Functions ----*/
void DisplayLoading(int No_ofHASH , int column){
  display.setCursor(column, 30);
  for(int i = 0 ; i < No_ofHASH ;i++){
    digitalWrite(12 , HIGH);
    display.print("#");
    display.display();
    delay(250);
    digitalWrite(12 , LOW);
    delay(250);
  }
}
void DisplayIsUploading(){
 /*Delay Display until the uploading ends*/
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 16);
  // Display static text
  display.println("The Code is uploading ");
  display.display();   
}


void DisplayFlashed(){
  /*Display that the code Has been Flashed Successfully*/
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(3, 16);
  // Display static text
  display.println("The Code Has been   Flashed Successfully!");
  display.display(); 
}
