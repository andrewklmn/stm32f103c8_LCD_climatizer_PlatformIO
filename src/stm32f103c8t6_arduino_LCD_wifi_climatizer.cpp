/*
  Home Climatizer 0.1
*/
#include "Wire.h"
#include "SimpleDHT.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include "Value_stack.h"
#include "On_off_driver.h"

#define MQ135_ANALOG_PIN  A0        // PA_0 as MQ-125 analog sensor for CO2
#define LED1              LED_BUILTIN
#define HEATER            A7
#define WATER             A6
#define WARNING_LED       A5
#define DANGER_LED        A4
#define BUTTON            A3
// LCD 2004 i2c PINS:
// i2c SDA  - B7
// i2c SCL  - B6

// for DHT11,
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int   pinDHT11 =           A2;  // PA_2 as HDT11 sensor for TEMP and Humidity


SimpleDHT11 dht11(pinDHT11);

Value_stack CO2_PPM_stack;

On_off_driver heater(10);
On_off_driver water(5);

byte temperature = 0;
byte humidity = 0;

// FOR GREEN ONE - small
const byte target_temp = 21;
const byte comfort_temp = 20;
const byte target_humidity = 50;

/*
// FOR RED ONE - big
const byte target_temp = 20;
const byte comfort_temp = 19;
const byte target_humidity = 50;
*/

int pass_adc_reading_cycles = 10;
int err = SimpleDHTErrSuccess;

int monitor_mode = 0;
int sensorValue = 0;

LiquidCrystal_I2C  screen1(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack GREEN
//LiquidCrystal_I2C  screen1(0x3F,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack RED

int MQ135_ao_from_adc_to_ppm(int ADC_value, int temp_value) {




  // FOR GREEN ONE
  #define RLOAD 1000
  /// Calibration resistance at atmospheric CO2 level
  #define RZERO 53000
  /// Parameters for calculating ppm of CO2 from sensor resistance
  #define PARA 76.
  #define PARB 1.9


  /*
    // FOR RED ONE
    #define RLOAD 1000
    /// Calibration resistance at atmospheric CO2 level
    #define RZERO 71000
    /// Parameters for calculating ppm of CO2 from sensor resistance
    #define PARA 116
    #define PARB 2.8
    */


    float Up = 5.0;
    float Uadc_max = 3.3;
    int ADC_steps = 1023;

    float Uadc = Uadc_max/ADC_steps*ADC_value;
    float resistance = (Up/Uadc)*RLOAD - RLOAD;
    return PARA * pow((resistance/RZERO), -PARB);
};

void setup() {

    pinMode(LED1, OUTPUT);

    pinMode(HEATER, OUTPUT);
    digitalWrite(HEATER, LOW);

    pinMode(WATER, OUTPUT);
    digitalWrite(WATER, LOW);

    pinMode(WARNING_LED, OUTPUT);
    digitalWrite(WARNING_LED, HIGH);

    pinMode(DANGER_LED, OUTPUT);
    digitalWrite(DANGER_LED, LOW);

    pinMode(BUTTON, INPUT);

    //Serial.begin(9600);
    //while (!Serial);
    //Serial.println("Home Climatizer 0.1");

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

    delay(400);

    digitalWrite(WARNING_LED, LOW);
    digitalWrite(DANGER_LED, HIGH);

    delay(350);

    digitalWrite(WARNING_LED, LOW);
    digitalWrite(DANGER_LED, LOW);

}

void loop() {

        delay(750);
        digitalWrite(LED1, HIGH);


        // check button state
        if ( digitalRead(BUTTON) == HIGH ) {
          // change current working mode
          if (monitor_mode == 0) {
            monitor_mode = 1;
            screen1.setCursor(0,3);
            screen1.print("----monitor mode----");
            //heater.stop();
            //water.stop();
          } else {
            monitor_mode = 0;
            screen1.setCursor(0,3);
            screen1.print("Heater:--- Water:---");
          };
        };

        heater.tic_tac();
        water.tic_tac();


        //dht11.read(&temperature, &humidity, NULL);
        if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
          //Serial.print("Read DHT11 failed, err=");
          //Serial.println(err);
          temperature=0;
          humidity=0;
          //return;
        } else {
          temperature = temperature-1;
        };

        screen1.setCursor(5,0);
        screen1.print("+");
        screen1.print(temperature);
        screen1.print((char)223);
        screen1.print("C ");

        screen1.setCursor(16,0);
        screen1.print(humidity);
        screen1.print("%  ");

    if ( monitor_mode == 0) {

        screen1.setCursor(0,2);
        if (temperature == 0 && humidity == 0) {
          screen1.print(" DHT11 Sensor Error!");
          heater.set_state(1);
          water.set_state(1);
        } else if (temperature > target_temp && humidity > target_humidity) {
          screen1.print("  Comfort condition ");
          heater.set_state(0);
          water.set_state(0);
        } else if (temperature > target_temp && humidity <= target_humidity) {
          screen1.print("      Too dry!      ");
          heater.set_state(0);
          water.set_state(1);
        } else if (temperature <= target_temp && humidity <= target_humidity){
          if (temperature >= comfort_temp) {
            screen1.print("      Too dry!      ");
          } else {
            screen1.print(" Too cold! Too dry! ");
          };
          heater.set_state(1);
          water.set_state(1);
        } else if (temperature <= target_temp && humidity > target_humidity){
          if (temperature >= comfort_temp) {
            screen1.print("  Normal condition  ");
          } else {
            screen1.print("     Too cold!      ");
          };
          heater.set_state(1);
          water.set_state(0);
        } else {
          screen1.print("  Normal condition  ");
        };


        if(heater.get_state()==0) {
          digitalWrite(HEATER, LOW);
          screen1.setCursor(7,3);
          screen1.print("off");
        } else {
          digitalWrite(HEATER, HIGH);
          screen1.setCursor(7,3);
          screen1.print("ON ");
        };


        if(water.get_state()==0) {
          digitalWrite(WATER, LOW);
          screen1.setCursor(17,3);
          screen1.print("off");
        } else {
          digitalWrite(WATER, HIGH);
          screen1.setCursor(17,3);
          screen1.print("ON ");
        };
    } else {

      digitalWrite(HEATER, LOW);
      digitalWrite(WATER, LOW);
      screen1.setCursor(0,2);
      screen1.print("                    ");
      //heater.set_state(0);
      //water.set_state(0);
    };

        delay(750);
        digitalWrite(LED1, LOW);



      if (pass_adc_reading_cycles == 0) {
          int analog_value = (int)analogRead(MQ135_ANALOG_PIN);
          CO2_PPM_stack.add_value(MQ135_ao_from_adc_to_ppm(analog_value, temperature));
          sensorValue = CO2_PPM_stack.get_average();
          //int sensorValue = analog_value;
          if (sensorValue > 9999) sensorValue = 9999;
          screen1.setCursor(12,1);
          if (sensorValue > 999) {
            screen1.print(sensorValue);
            screen1.print("ppm ");
          } else {
            screen1.print(sensorValue);
            screen1.print("ppm  ");
          };
      }  else {
          pass_adc_reading_cycles--;
      };


      //if ( monitor_mode == 0) {
          if (sensorValue > 1000 && sensorValue <= 1500) {
            digitalWrite(WARNING_LED, HIGH);
            digitalWrite(DANGER_LED, LOW);
              screen1.setCursor(0,2);
              screen1.print("  Need ventilation! ");
          } else if (sensorValue > 1500){
            digitalWrite(WARNING_LED, LOW);
            digitalWrite(DANGER_LED, HIGH);
              screen1.setCursor(0,2);
              screen1.print("  NEED VENTILATION! ");
          } else {
            digitalWrite(WARNING_LED, LOW);
            digitalWrite(DANGER_LED, LOW);
          };
      //};

};
