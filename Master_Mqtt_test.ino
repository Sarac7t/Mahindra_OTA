#include <WiFi.h>
#include <String.h>
#include <ArduinoJson.h>
//#include <PubSubClient.h>
#include <MQTTPubSubClient_Generic.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include <Preferences.h>

String ssid_e;
String password_e;
String macs = "";
unsigned long lastKeepalive=90000;
bool versionflag = false;
String FirmwareVer = {
  "1.0"
};
int count=0;

#define SECOND 1000
#define MINUTE 60 * SECOND
#define URL_fw_Version "https://github.com/Sarac7t/Mahindra_Aiera/raw/main/version.txt"
#define URL_fw_Bin "https://github.com/Sarac7t/Mahindra_Aiera/raw/main/fw.bin"

WiFiClientSecure client;
MQTTPubSubClient mqttClient;
Preferences preferences;

const char* mqtt_server = "27537ca038a046b0baaa55aa6884d1da.s1.eu.hivemq.cloud";
const char* mqtt_User = "Saravana";
const char* mqtt_Pwd = "Aiera@123";
const String Room_Status = "Aiera/Fw_Details";
const String PublishTopic = "Aiera/Update";
const String Room_OTA = "Aiera/Fw_Update";
const String Room_Control = "Aiera/Control";
void firmwareUpdate();
int FirmwareVersionCheck();

void callback(char* topic, byte* payload, unsigned int length);

String getMacAddress() {
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = { 0 };
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    return String(baseMacChr);
}
void reconnect() {
 Serial.println("Reconnecting...");
  while (!client.connected())
  { 
count=0;
   WiFi.begin(ssid_e.c_str(), password_e.c_str());
delay(500);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    count=count+1;
    if(count>=30)  ESP.restart();
    }
      
   
    randomSeed(micros());
  client.setInsecure();
    while (!client.connect(mqtt_server, 8883))
  {
    Serial.print(".");
    delay(1000);
  }
   mqttClient.begin(client);
   Serial.print("Connecting to mqtt broker...");
    while (!mqttClient.connect("espClient","Room217", "Room217aiera"))
  {
    Serial.print(".");
    delay(1000);
        count=count+1;
    if(count>=40)  ESP.restart();
  }

//  Serial.println(" connected!");
//     client.subscribe(Room_Status);
////      client.subscribe(PublishTopic);
//        client.subscribe(Room_OTA);
//      client.subscribe(Room_Control);
 
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("WiFi connected");
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
    macs = getMacAddress();
 Serial.print(macs);
  mqttClient.subscribe(Room_OTA, [](const String & payload, const size_t size)
  {
    (void) size;
  Serial.print("Message arrived [");
  Serial.print(Room_OTA);
  Serial.print("] ");
//if (strcmp(topic, Room_OTA) == 0) {{"AieraId":"SAC02,"Data_Topics":"TurnOn,"StatusId":"ON"}
//  if (topic== "Aiera/Fw_Update") {
    const size_t capacity = JSON_OBJECT_SIZE(11) + 240;
    DynamicJsonDocument root(1024);
    deserializeJson(root, payload);
    String device = root["AieraId"].as<String>();
    String macid = getMacAddress();
    if (device == macid) {
      Serial.println(" mac id matched for devicedevice");
      Serial.println(device);
      if (FirmwareVersionCheck()) {
        firmwareUpdate();
      }
    }
  });
   mqttClient.subscribe(PublishTopic, [](const String & payload, const size_t size)
  {
        (void) size;
  Serial.print("Message arrived [");
  Serial.print(PublishTopic);
  Serial.print("] ");
//  else if (strcmp(topic, Room_Control) == 0) {
//   else if (topic== "Aiera/Control") {
    const size_t capacity = JSON_OBJECT_SIZE(11) + 240;
    DynamicJsonDocument root(1024);
    deserializeJson(root, payload);
    String device = root["RoomId"].as<String>();
    String datas = root["Controls"].as<String>();
    String macid = getMacAddress();
    if (device == macid) {
      Serial.println(" mac id matched for devicedevice");
      Serial.println(device);


    
    }
  });
     mqttClient.subscribe(Room_Status, [](const String & payload, const size_t size)
  {
        (void) size;
  Serial.print("Message arrived [");
  Serial.print(Room_Status);
  Serial.print("] ");
//  else if (strcmp(topic, Room_Status) == 0) {
// else if (topic== "Aiera/Update") {
    const size_t capacity = JSON_OBJECT_SIZE(11) + 240;
    DynamicJsonDocument root(1024);
    deserializeJson(root, payload);
    String device = root["metabinId"].as<String>();
    String macid = getMacAddress();
    if (device == macid) {
      Serial.println(" mac id matched for devicedevice");
      Serial.println(device);
      
    }
  

});

lastKeepalive= MINUTE * 2;
}

void publish_data(int state, String Mystring) {

  Serial.println("push data");
   Serial.println(Mystring);

  String payload = "{";
  payload += "\"RoomId\":\"";
  payload += getMacAddress();
  payload += "\",";
  payload += "\"Room_datas\":\"";
  payload += Mystring;
  payload += "\",";
  payload += "\"IP address\":\"";
  payload += WiFi.localIP().toString();
  payload += "\"";
  payload += "}";
  char attributes[800];
  payload.toCharArray(attributes, 800);
  mqttClient.publish(Room_Status, attributes);
  Serial.println(attributes);
  Serial.println("Data sent successfully");
}
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
//preferences.begin("credentials", false);
//  ssid_e = preferences.getString("ssid", ""); 
//  password_e = preferences.getString("password", "");
    ssid_e = "Circuit-7"; 
  password_e = "Dexter007"; 
     Serial.println(ssid_e);
   Serial.println(password_e);
   WiFi.begin(ssid_e.c_str(), password_e.c_str());
reconnect();

}
void firmwareUpdate(void) {
  WiFiClient client;
  //  client.setCACert(rootCACertificate);
  httpUpdate.setLedPin(2, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClient client1;
  //  std::unique_ptr<BearSSL::WiFiClientSecure>client1(new BearSSL::WiFiClientSecure);
  //   client1->setInsecure();

  //  if (client1)
  //  {
  //    client1.setCACert(rootCACertificate);
  //    client -> setInsecure();
  // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
  HTTPClient https;
  if (https.begin(client1, fwurl)) {  // HTTPS
    Serial.print("[HTTPS] GET...\n");
    // start connection and send HTTP header
    delay(100);
    httpCode = https.GET();
    delay(200);
    if (httpCode == HTTP_CODE_OK)  // if version received
    {
      payload = https.getString();  // save received version
    } else {
      Serial.print("error in downloading version file:");
      Serial.println(httpCode);
    }
    https.end();
  }
  //  }
  if (httpCode == HTTP_CODE_OK)  // if version received
  {
    payload.trim();
    versionflag = true;
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } else {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  }
  return 0;
}
void loop() {
mqttClient.update();
if(WiFi.status() != WL_CONNECTED)
{
  reconnect();
  }
 if (millis() - lastKeepalive > MINUTE * 2) {
    
    publish_data(1, "11110000");
    lastKeepalive = millis();
   
   delay(1000);
  
  } 

}
