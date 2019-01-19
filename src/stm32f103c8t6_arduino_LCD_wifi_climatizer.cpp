/*
  Home Climatizer 0.1
*/
#include "Wire.h"
#include "SimpleDHT.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include "Value_stack.h"

// for DHT11,
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2

int pinDHT11 = A2;                        // PA_2 as HDT11 sensor for TEMP and Humidity
SimpleDHT11 dht11(pinDHT11);
#define MQ135_ANALOG_PIN        A0        // PA_0 as MQ-125 analog sensor for CO2

Value_stack CO2_PPM_stack;

int heater = 0;
int water = 0;

byte temperature = 0;
byte target_temp = 20;
byte comfort_temp = 18;

byte humidity = 0;
byte target_humidity = 40;

int pass_adc_reading_cycles = 10;

int err = SimpleDHTErrSuccess;

LiquidCrystal_I2C  screen1(0x3F,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack

int MQ135_ao_from_adc_to_ppm(int adc_value, int temp_value) {

    float MQ135_SCALINGFACTOR = 116;
    float MQ135_EXPONENT = -2.739;
    float Up = 5.0;
    float Uadc = 3.3;
    int ADC_steps = 1024;
    int R2 = 1000;
    float ro = 41763;              // Ro MQ135

    if (temp_value < 8 ) temp_value=8;
    //if (temp_value > 20 ) temp_value=20;

    int temp_correction = (temp_value-20)*3.5;

    float Ud = (float)Uadc/ADC_steps * (float)(adc_value - temp_correction);
    float resvalue = Up/Ud*R2-R2;  // Rs MQ135 now
    return round((float)MQ135_SCALINGFACTOR * pow( ((float)resvalue/ro), MQ135_EXPONENT));
};

void setup() {

    Serial.begin(9600);
    while (!Serial);
    Serial.println("Home Climatizer 0.1");

    screen1.begin (20,4);
    screen1.setBacklightPin(3,POSITIVE);
    screen1.setBacklight(HIGH);

    screen1.home (); // set cursor to 0,0
    screen1.print("Temp:----   Hum:----");
    screen1.setCursor(0,1);
    screen1.print(" CO2 level: wait...");
    //screen1.print(" CO2: ---");
    screen1.setCursor(0,2);
    screen1.print(" System is starting ");
    screen1.setCursor(0,3);
    screen1.print("Heater:--- Water:---");
    delay(750);
}

void loop() {

        delay(750);

        //dht11.read(&temperature, &humidity, NULL);

        if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
          //Serial.print("Read DHT11 failed, err=");
          //Serial.println(err);
          temperature=0;
          humidity=0;
          //return;
        } else {
          //temperature = temperature-2;
        };

        screen1.setCursor(5,0);
        screen1.print("+");
        screen1.print(temperature);
        screen1.print((char)223);
        screen1.print("C ");

        screen1.setCursor(16,0);
        screen1.print(humidity);
        screen1.print("%  ");

        screen1.setCursor(0,2);
        if (temperature == 0 && humidity == 0) {
          screen1.print(" Temp Sensor Error!");
          heater = 0;
          water = 0;
        } else if (temperature >= target_temp && humidity >= target_humidity) {
          screen1.print(" Comfort condition  ");
          heater = 0;
          water = 0;
        } else if (temperature >= target_temp && humidity < target_humidity) {
          screen1.print("      Too dry!      ");
          heater = 0;
          water = 1;

        } else if (temperature < target_temp && humidity < target_humidity){

          if (temperature >= comfort_temp) {
            screen1.print("      Too dry!      ");
          } else {
            screen1.print(" Too cold! Too dry! ");
          };
          heater = 1;
          water = 1;
        } else if (temperature < target_temp && humidity >= target_humidity){
          if (temperature >= comfort_temp) {
            screen1.print("  Normal condition  ");
          } else {
            screen1.print("     Too cold!      ");
          };
          heater = 1;
          water = 0;
        };

        if (heater==0) {
          screen1.setCursor(7,3);
          screen1.print("off");
        } else {
          screen1.setCursor(7,3);
          screen1.print("ON ");
        };

        if (water==0) {
          screen1.setCursor(17,3);
          screen1.print("off");
        } else {
          screen1.setCursor(17,3);
          screen1.print("ON ");
        };

        delay(750);

        int analog_value = (int)analogRead(MQ135_ANALOG_PIN);
        CO2_PPM_stack.add_value(MQ135_ao_from_adc_to_ppm(analog_value, temperature));
        int sensorValue = CO2_PPM_stack.get_average();
        //int sensorValue = analog_value;

        if (pass_adc_reading_cycles == 0) {

          screen1.setCursor(12,1);
          if (sensorValue > 999) {
            screen1.print(sensorValue);
            screen1.print("ppm ");
            //if (sensorValue > 1000) {
              screen1.setCursor(0,2);
              screen1.print("  Need ventilation! ");
            //};
          } else {
            screen1.print(sensorValue);
            screen1.print("ppm  ");
          };
        } else {
          pass_adc_reading_cycles--;
        };
}
