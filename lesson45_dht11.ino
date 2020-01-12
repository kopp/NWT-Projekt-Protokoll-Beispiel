/* DHT11 testen
 *  DHT11 an D8 (mit 5kOhm Pullup).
 */

#include "DHT_U.h"
#include "DHT.h"
#include <stdio.h>

const int DHTPIN = 8;

DHT dht(DHTPIN, DHT11);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

char time_format_buffer[20];

void loop() {
  delay(2000);

  unsigned long uptime_ms = millis();
  unsigned int uptime_s = uptime_ms/1000;
  float relative_humidity = dht.readHumidity();
  float temperature_in_C = dht.readTemperature();

  if (isnan(relative_humidity) || isnan(temperature_in_C)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // sprintf: https://forum.arduino.cc/index.php?topic=43562.0
  sprintf(time_format_buffer, "Zeit: %5d s", uptime_s);

  Serial.print(time_format_buffer);
  Serial.print(F("; Temperatur: "));
  Serial.print(temperature_in_C);
  Serial.print(F("°C; rel. Luftfeuchtigkeit: "));
  Serial.print(relative_humidity);
  Serial.println(F("%"));
}
