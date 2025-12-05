#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>   
#include <Arduino.h>

// Server configuratie
#define serverIP   "192.168.0.35"   // Pi IP
#define serverPort 5001

// API-sleutel
#define API_KEY "E2R8T9" // Persoonlijke API-sleutel
#define dataset "test"


// WiFi netwerk credentials
#define SECRET_SSID "E109-E110"
#define SECRET_PASS "DBHaacht24"

// MQTT-config
#define BROKER   "192.168.0.150"
#define MQTTPORT 1883
#define TOPIC    "examen"
#define MSG_LEN  512

#define Buzzer 27
#define LedG 21
#define LedO 22
#define LedR 23


WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);


void setup() {
  String Waarde = "";

  // pinmodes defenieren
  pinMode(LedG, OUTPUT);
  pinMode(LedO, OUTPUT);
  pinMode(LedR, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  // voor de zekerheid gaan we alles nog eerst uitzetten
  digitalWrite(LedG, LOW);
  digitalWrite(LedO, LOW);
  digitalWrite(LedR, LOW);
  digitalWrite(Buzzer, LOW);

  // Seriële communicatie starten
  Serial.begin(115200);
  Serial.print("________________________________________________\n");
  Serial.print("               Start van programma              \n");
  Serial.print("________________________________________________\n");
  // Wifi opzetten
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Serial.print("Verbinden met WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Geen Verbinding met Wifi\n");
  }
  Serial.println("\nVerbonden!");
  Serial.print("________________________________________________\n");

    // MQTT verbinden
  mqttClient.setId("energy-esp32");
  Serial.println("MQTT verbinden...");
  while (!mqttClient.connect(BROKER, MQTTPORT)) {
    Serial.print("MQTT fout: ");
    Serial.println(mqttClient.connectError());
    delay(1000);
  }
  Serial.println("MQTT verbonden.");
  Serial.print("________________________________________________\n\n");
}

void loop() {
   
    // MQTT-verbinding levend houden
  mqttClient.poll();

    // gegevens van alle studenten ophalen
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String("http://") + serverIP + ":" + serverPort + "/api/energy/next?dataset=" + dataset + "&apikey=" + API_KEY;
      http.begin(url);

      // HTTP GET verzoek sturen
      int httpResponseCode = http.GET();
      if (httpResponseCode == 200) {
        String payload = http.getString();
        Serial.println("Ontvangen JSON:");
        Serial.println(payload);

        // JSON parseren
        const size_t capacity = 256;
        DynamicJsonDocument doc(capacity);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // gegevens uitlezen
          float active_current_a               = doc["active_current_a"];
          float active_power_average_w         = doc["active_power_average_w"];
          float active_power_w                 = doc["active_power_w"];
          float active_voltage_l1_v            = doc["active_voltage_l1_v"];
          String montly_power_peak_timestamp   = doc["montly_power_peak_timestamp"];
          float montly_power_peak_w            = doc["montly_power_peak_w"];
          String timestamp                     = doc["timestamp"];
          float total_gas_m3                   = doc["total_gas_m3"];
          float total_power_export_kwh         = doc["total_power_export_kwh"];
          float total_power_import_kwh         = doc["total_power_import_kwh"];
          int index                            = doc["index"];


          // gegevens printen
          Serial.printf(
            "active_current_a: %.1f\nactive_power_average_w: %.1f\nactive_power_w: %.0f\nactive_voltage_l1_v: %.0f\nmontly_power_peak_timestamp: %.1f\nmontly_power_peak_w: %.1f\ntimestamp: %.0f\ntotal_gas_m3: %.0f\ntotal_power_import_kwh: %.0f\ntotal_power_import_kwh: %.0f\nindex: %d\n",
            active_current_a, active_power_average_w, active_power_w, active_voltage_l1_v,
            montly_power_peak_timestamp, montly_power_peak_w, timestamp, total_gas_m3, total_power_export_kwh, total_power_import_kwh, index
          );
          Serial.print("________________________________________________\n");

          // alles printen
          if (index >= 0) {
            Serial.printf("waarde: active_current_a            = %.2f\n", active_current_a );
            Serial.printf("waarde: active_power_average_w      = %.2f\n", active_power_average_w);
            Serial.printf("waarde: active_power_w              = %.2f\n", active_power_w);
            Serial.printf("waarde: active_voltage_l1_v         = %.2f\n", active_voltage_l1_v);
            Serial.printf("waarde: montly_power_peak_timestamp = %.2f\n", montly_power_peak_timestamp);
            Serial.printf("waarde: montly_power_peak_w         = %.2f\n", montly_power_peak_w);
            Serial.printf("waarde: timestamp                   = %.2f\n", timestamp);
            Serial.printf("waarde: total_gas_m3                = %.2f\n", total_gas_m3);
            Serial.printf("waarde: total_power_export_kwh      = %.2f\n", total_power_export_kwh);
            Serial.printf("waarde: total_power_import_kwh      = %.2f\n", total_power_import_kwh);
            Serial.printf("waarde: index                       = %d\n", index);
            Serial.print("________________________________________________\n\n");

          }

          // kijken welke waardes we hebben
          if (active_power_w < 300){
              int waarde = active_power_w;
              digitalWrite(LedG, HIGH);
              digitalWrite(LedO, LOW);
              digitalWrite(LedR, LOW);
              digitalWrite(Buzzer, LOW);
          }
          else if (500 > active_power_w >= 300) {
              digitalWrite(LedG, LOW);
              digitalWrite(LedO, HIGH);
              digitalWrite(LedR, LOW);
              digitalWrite(Buzzer, LOW);
          }
          else if (800 > active_power_w >= 500) {
              digitalWrite(LedG, LOW);
              digitalWrite(LedO, LOW);
              digitalWrite(LedR, HIGH);
              digitalWrite(Buzzer, LOW);
          }
          else if (active_power_w >= 800) {
              digitalWrite(LedG, LOW);
              digitalWrite(LedO, LOW);
              digitalWrite(LedR, HIGH);
              digitalWrite(Buzzer, HIGH);
          }

          //  JSON sturen
          StaticJsonDocument<256> out;
          out["Stroom"]     = active_current_a;
          out["Gem_Vermoge"]= active_power_average_w;
          out["Vermoge"]    = active_power_w;
          out["Spanning"]   = active_voltage_l1_v;
          out["Peak_Tijd"]  = montly_power_peak_timestamp;
          out["MonthPeak"]  = montly_power_peak_w;
          out["Time"]       = timestamp;
          out["Exp"]        = total_power_export_kwh;
          out["Imp"]        = total_power_import_kwh;
          out["index"]      = index;
        
          char buf[MSG_LEN];
          size_t len = serializeJson(out, buf, sizeof(buf));

          Serial.print("MQTT JSON: ");
          Serial.write(buf, len);
          Serial.println();

          mqttClient.beginMessage(TOPIC);
          mqttClient.write((const uint8_t*)buf, len);
          mqttClient.endMessage();
        }

      // foutafhandeling
      } else {
        Serial.printf("HTTP foutcode: %d\n", httpResponseCode);
      }

      http.end();
    }

  // laatse bericht als we klaar zijn
  Serial.println("Alle records opgehaald.");
  Serial.print("================================================\n\n");
  delay(1000);   
} 

/*
// functie om reset uit te voeren
void reset(){
  if (keuze == "reset"){
    String url = String("http://") + serverIP + ":" + serverPort + "/api/energy/reset?dataset=" + dataset + "&apikey=" + API_KEY;
    http.begin(url);
  }

}
*/