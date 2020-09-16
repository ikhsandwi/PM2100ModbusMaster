/*
File     : RS485_HalfDuplex_PM2100_antares.ino
Author   : Ikhsan Dwi
Version  : 1.1
Create   : 9-09-2020
last update : 16-09-2020
*/

/*  Wiring
PM2100    <==>    Modul RS485
 A        <==>        A
 B        <==>        B

Modul RS485     <==>    ESP32 Board
    VCC         <==>        VIN
    GND         <==>        GND
    DI          <==>        TX
    RO          <==>        RX
    DE          <==>        18
    RE          <==>        19
*/

#include <ModbusMaster.h>                                 //modbus master library
#include <AntaresESP32MQTT.h>                             //ESP32 MQTT Antares Library
#include <WiFi.h>                                         //WiFi library
#include <WiFiClient.h>                                   //WiFi client library

// Your WiFi credentials.
// Set password to "" for open networks.
#define ACCESSKEY "a08c0faa1bxxxxxx:9ae97d4deexxxxxx"     //access token user
#define WIFISSID "..."                                    //SSID wifi
#define PASSWORD "..."                                    //password wifi

#define projectName "5f0e7893658acc00108aa821"            //project name
#define deviceName "1598795191613845242"                  //device name

AntaresESP32MQTT antares(ACCESSKEY);                      //authentification access token to antares

#define MAX485_DE      18                                 // konfigurasi pin DE modul modbus ke pin ESP32
#define MAX485_RE_NEG  19                                 // konfigurasi pin RE modul modbus ke pin ESP32

// instantiate ModbusMaster object
ModbusMaster node;                                        // inisialisasi ModbusMaster

//variable
bool state = true;
uint16_t rawData,resultVB,resultVC;
int16_t VA[2],VB[2],VC[2];
float VL1,VL2,VL3;
unsigned long *decoderVA,*decoderVB,*decoderVC;


void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void setup()
{
  Serial.begin(9600);
  
  //Setup WiFi connection
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)                     // reconnecting jaringan WiFi
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");                         // tersambung dengan jaringan Wifi 

  antares.setDebug(true);                                   // status debugging
  antares.wifiConnection(WIFISSID, PASSWORD);               // cek koneksi wifi untuk menghubungkan ke server antares
  antares.setMqttServer();                                  // memeriksa ke MQTT server antares
  
  pinMode(MAX485_RE_NEG, OUTPUT);                           // inisialisasi pin RE sebagai output 
  pinMode(MAX485_DE, OUTPUT);                               // inisialisasi pin DE sebagai output
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);                           // set pin RE low
  digitalWrite(MAX485_DE, 0);                               // set pin DE low

  // set Modbus slave ID nomor 1 dengan komunikasi serial
  node.begin(1, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}


void loop()
{
  rawData = node.readHoldingRegisters(3027, 6);             // read data address modbus dimulai dari adddress 3027 dengan panjang pembacaan 6 address dari address tersebut
                                                            // address yang diambil address (n-1)
  if (rawData == node.ku8MBSuccess)                         // kondisi jika pembacaan data modbus success
  {
    VA[0]=node.getResponseBuffer(0);                        // nilai address pada array ke 0 adalah nilai dari address 3027 sebagai nilai VA[0]
    VA[1]=node.getResponseBuffer(1);                        // nilai address pada array ke 1 adalah nilai dari address 3028 sebagai nilai VA[1]
    VB[0]=node.getResponseBuffer(2);                        // nilai address pada array ke 2 adalah nilai dari address 3029 sebagai nilai VB[0]
    VB[1]=node.getResponseBuffer(3);                        // nilai address pada array ke 3 adalah nilai dari address 3030 sebagai nilai VB[1]
    VC[0]=node.getResponseBuffer(4);                        // nilai address pada array ke 4 adalah nilai dari address 3031 sebagai nilai VC[0]
    VC[1]=node.getResponseBuffer(5);                        // nilai address pada array ke 5 adalah nilai dari address 3027 sebagai nilai VC[1]
    
    // karena nilai variable terdiri dari 2 address maka dilakukan penggabungan antara 2 address yang bersangkutan
    decoderVA = (unsigned long*)&VL1;                       // pointer decoderVA sebagai nilai dari VL1
    *decoderVA = (unsigned long)VA[0]<<16 | VA[1];          // penggabungan 2 variable int16 dan mengkonversi menjadi sebuah data float ke variable VL1
    decoderVB = (unsigned long*)&VL2;
    *decoderVB = (unsigned long)VB[0]<<16 | VB[1];
    decoderVC = (unsigned long*)&VL3;
    *decoderVC = (unsigned long)VC[0]<<16 | VC[1];
  }
  // cek koneksi ke MQTT antares
  antares.checkMqttConnection();
  // print data hasil penggabungan dan konversi diatas
  Serial.print("Volt Line AN = ");  Serial.print(VL1,1);
  Serial.print("\t Volt Line BN = "); Serial.print(VL2,1);
  Serial.print("\t Volt Line CN = "); Serial.print(VL3,1);
  Serial.println();
  
  // data yang akan dikirim ke antares
  antares.add("VA",VL1);                                    
  antares.add("VB",VL2);
  antares.add("VC",VL3);
  // publish data diatas ke platform antares
  antares.publish(projectName, deviceName);
  delay(5000);
}
