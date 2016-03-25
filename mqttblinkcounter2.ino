

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <math.h>

//#define INTERRUPT_INPUT 15
#define INTERRUPT_INPUT 15
#define LED_PIN 0
#define DEBUG_OUTPUT 0

volatile int pulse_counter = 0;
int cooldown = 30;
volatile long lastcount = millis();
long lastReconnectAttempt = 0;
long reconnects = 0;

const char* ssid = "xxx";
const char* password = "xxx";
 
const char* topic = "esp8266_arduino_out";
//const char* server = "192.168.178.20";
const char* server = "xxx";

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}
 
WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);
 

 
 
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

 void interrupt_handler()
{

  
  if(millis() - lastcount > cooldown)
  {
    pulse_counter = pulse_counter + 1;
    lastcount = millis();  
  }

  
}
void setup() {

 if(DEBUG_OUTPUT){
  Serial.begin(115200);
 }
  delay(10);
  
  Serial.print("Attaching interrupt...to");
    Serial.println(INTERRUPT_INPUT);

  //  digitalWrite(INTERRUPT_INPUT, HIGH);
  attachInterrupt(INTERRUPT_INPUT,
                  interrupt_handler,
                  FALLING);
  
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  // Generate client name based on MAC address and last 8 bits of microsecond counter
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
 
  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);
  delay(2000);
  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Topic is: ");
    Serial.println(topic);
    
    if (client.publish(topic, "hello from ESP8266")) {
      Serial.println("Publish ok");
            client.subscribe("power_usage");

    }
    else {
      Serial.println("Publish failed");
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    
    abort();
  }
}
 
void loop() {
  static int counter = 0;
  
/*  String payload = "{\"micros\":";
  payload += micros();
  payload += ",\"counter\":";
  payload += pulse_counter;
 */

String payload = "pulses:";
//payload+=pulse_counter; 
  payload += pulse_counter;
  

  if (!client.connected()) {
    Serial.println("Client Disconnected....");
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
            Serial.println("Connection Reestablished....");
        lastReconnectAttempt = 0;
      }
    }
  }

   
   if (client.connected()){
    Serial.print("Sending payload: ");
    Serial.println(payload);
    
    if (client.publish(topic, (char*) payload.c_str())) {
      Serial.println("Publish ok");
      pulse_counter=0;
      digitalWrite(LED_PIN,LOW);
      delay(50);
      digitalWrite(LED_PIN, HIGH);
    }
   
  }
  
  ++counter;
  client.loop();
  delay(5000);
}


boolean reconnect() {
  if (client.connect("solarCounter")) {
    // Once connected, publish an announcement...
    client.publish(topic, "hello from ESP8266");
    // ... and resubscribe
    //client.subscribe("inTopic");
  }
  return client.connected();
}


