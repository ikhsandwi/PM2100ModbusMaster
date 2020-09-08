#include <ModbusMaster.h>
#include <AntaresESP32MQTT.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define ACCESSKEY "a08c0faa1b988b11:9ae97d4dee93a723"
#define WIFISSID "takkusangka"
#define PASSWORD "anakrantau"

#define projectName "5f0e7893658acc00108aa821"
#define deviceName "1598795191613845242"

AntaresESP32MQTT antares(ACCESSKEY);

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "takkusangka";
char pass[] = "anakrantau";

#define MAX485_DE      18
#define MAX485_RE_NEG  19

// instantiate ModbusMaster object
ModbusMaster node;

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
  
  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");  

  antares.setDebug(true);
  antares.wifiConnection(WIFISSID, PASSWORD);
  antares.setMqttServer();
  
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);  

  // Modbus slave ID 1
  node.begin(1, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  
}


void loop()
{
  rawData = node.readHoldingRegisters(3027, 6); //voltage AN
  
  if (rawData == node.ku8MBSuccess)
  {
    VA[0]=node.getResponseBuffer(0);
    VA[1]=node.getResponseBuffer(1);
    VB[0]=node.getResponseBuffer(2);
    VB[1]=node.getResponseBuffer(3);
    VC[0]=node.getResponseBuffer(4);
    VC[1]=node.getResponseBuffer(5);
    decoderVA = (unsigned long*)&VL1;
    *decoderVA = (unsigned long)VA[0]<<16 | VA[1];
    decoderVB = (unsigned long*)&VL2;
    *decoderVB = (unsigned long)VB[0]<<16 | VB[1];
    decoderVC = (unsigned long*)&VL3;
    *decoderVC = (unsigned long)VC[0]<<16 | VC[1];
  }
  antares.checkMqttConnection();
  Serial.print("Volt Line AN = ");  Serial.print(VL1,1);
  Serial.print("\t Volt Line BN = "); Serial.print(VL2,1);
  Serial.print("\t Volt Line CN = "); Serial.print(VL3,1);
  Serial.println();

  antares.add("VA",VL1);
  antares.add("VB",VL2);
  antares.add("VC",VL3);
  antares.publish(projectName, deviceName);
  delay(5000);
}
