

#include <Bonezegei_DHT22.h>

//param = DHT22 signal pin
Bonezegei_DHT22 dht(4);

union FloatUnion {
    float floatValue;
    uint32_t intRepresentation;
};

void setup() {
 for (int pin = 5; pin <= 8; pin++) {
    pinMode(pin, OUTPUT);
  }
 
  initAwal();
  Serial.begin(115200);
  //dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  dht.begin();
}

void initAwal()
{
  for (int pin = 5; pin <= 8; pin++) {
    digitalWrite(pin, LOW);  // Menyalakan LED pada pin saat ini
    delay(500);               // Menunggu selama 500 milidetik
    digitalWrite(pin, HIGH);   // Mematikan LED pada pin saat ini
  }
    
}

void konversiToLed(float temp)
{
  FloatUnion data;
    data.floatValue = temp;
    Serial.print("Nilai float: ");
    Serial.println(data.floatValue);
    Serial.print("Representasi biner: ");
    Serial.println(data.intRepresentation, BIN);
    Serial.print("Representasi heksadesimal: ");
    Serial.println(data.intRepresentation, HEX);
    uint32_t intRepresentation;
    memcpy(&intRepresentation, &temp, sizeof(float));
     for (int i = 0; i < 32; i += 4) {
        uint8_t nibble = (intRepresentation >> i) & 0x0F;
        digitalWrite(5, nibble & 0x01);
        digitalWrite(6, (nibble >> 1) & 0x01);
        digitalWrite(7, (nibble >> 2) & 0x01);
        digitalWrite(8, (nibble >> 3) & 0x01);
        delay(100); // Waktu tunda untuk memastikan data terbaca
    }
}

void loop() {
   if (dht.getData()) {                         // get All data from DHT22
    float tempDeg = dht.getTemperature();      // return temperature in celsius
    float tempFar = dht.getTemperature(true);  // return temperature in fahrenheit if true celsius of false
    int hum = dht.getHumidity();               // return humidity
    konversiToLed(tempDeg);
    String output = "Temperature: " + String(tempDeg, 1) + "°C ,  " + String(tempFar, 1) + "°F , Humidity: " + String(hum);
    Serial.println(output);
  }
  delay(2000);  
}