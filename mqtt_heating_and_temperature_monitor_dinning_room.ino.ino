#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

// Update these with values suitable for your network.
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int lastvalue = 0;
float temp = 0;
int inPin = 5;
int HeatingPin = 16;
String switch1;
String strTopic;
String strPayload;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);
  if(strTopic == "ha/switch1")
    {
    switch1 = String((char*)payload);
    if(switch1 == "ON")
      {
        Serial.println("ON");
        digitalWrite(HeatingPin, HIGH);
      }
    else
      {
        Serial.println("OFF");
        digitalWrite(HeatingPin, LOW);
      }
    }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("ha/#");
      value = digitalRead(inPin);
      if(value == 1)
        {
        client.publish("ha/heating","Off",1);
        }
      else
        {
          client.publish("ha/heating","On",1);
        }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  Serial.begin(115200);
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(inPin, INPUT);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
  digitalWrite(13, HIGH);

  pinMode(HeatingPin, OUTPUT);
  digitalWrite(HeatingPin, HIGH);
  
  sensors.begin();
}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  value = digitalRead(inPin);
  if(lastvalue != value)
    {
      if(value == 1)
        {
        client.publish("ha/heating","Off",1);
        digitalWrite(12, HIGH);
        //digitalWrite(15, HIGH);
        delay(100);
        digitalWrite(12, LOW);
        //digitalWrite(15, LOW);
        }
      else
        {
        client.publish("ha/heating","On",1);
        digitalWrite(12, HIGH);
        //digitalWrite(15, HIGH);
        delay(100);
        digitalWrite(12, LOW);
        //digitalWrite(15, LOW);        
        }
    Serial.println(value);        
    lastvalue = value;
    }
  long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    sensors.setResolution(12);
    sensors.requestTemperatures(); // Send the command to get temperatures
    temp = sensors.getTempCByIndex(0);
    Serial.println(temp);
    if((temp > 0) && (temp <60))
      {
      client.publish("ha/dinningroom_temperature", String(temp).c_str(),TRUE);
        digitalWrite(12, HIGH);
        //digitalWrite(15, HIGH);
        delay(100);
        digitalWrite(12, LOW);
        //digitalWrite(15, LOW);      
      }
  }

}
