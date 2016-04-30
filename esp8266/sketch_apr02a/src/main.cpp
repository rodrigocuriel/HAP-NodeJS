#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Curiel_NeoPixel.h>

const char* ssid = "viva-mexico-cast";
const char* password = "Xalapa7679Cuernavaca";
const char* mqtt_server = "curiel.me";

int hue = 0;
float brightness = 0.0;
float saturation = 0.0;

#define LED_PIN 2
#define NUM_LEDS 16

Curiel_NeoPixel strip = Curiel_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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
}

void setupOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("plantLight");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void changeHSB(){
  for(int pixelCount=0; pixelCount<strip.numPixels(); pixelCount++) {
    strip.setPixelColor(pixelCount, strip.HSVColor(hue,saturation,brightness));
  }
  strip.show();
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

  String payloadString = String((char*)payload);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  Serial.println("payload:" + payloadString);

  if (String(topic) == "moodLightPower" && String(payloadString) == "on"){
    hue = 0;
    saturation = 0.0;
    brightness = 100.0;
    changeHSB();
  }

  if (String(topic) == "moodLightPower" && String(payloadString) == "off"){
    hue = 0;
    saturation = 0.0;
    brightness = 0.0;
    changeHSB();
  }

  if (String(topic) == "moodLightBrightness"){
    brightness = (payloadString.toFloat())/100;
    changeHSB();
  }

  if (String(topic) == "moodLightHue"){
    hue = payloadString.toInt();
    changeHSB();
  }

  if (String(topic) == "moodLightSaturation"){
    saturation = (payloadString.toFloat())/100;
    changeHSB();

  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("moodLightPower");
      client.subscribe("moodLightBrightness");
      client.subscribe("moodLightHue");
      client.subscribe("moodLightSaturation");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  setup_wifi();
  setupOTA();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  ArduinoOTA.handle();
  client.loop();
  // colorWipe(strip.Color(255, 0, 0), 50); // Red
  // colorWipe(strip.Color(0, 255, 0), 50); // Green
  // colorWipe(strip.Color(0, 0, 255), 50); // Blue
}
