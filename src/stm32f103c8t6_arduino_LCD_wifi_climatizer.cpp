
/*
  Home Climatizer 0.3
*/

#define LIVING_ROOM 1
#define BEDROOM 2
#define YELLOW_BEDROOM 3
//======================= current location of device ========================
#define LOCATION BEDROOM


#include "Wire.h"
#include "SimpleDHT.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include "Value_stack.h"
#include "On_off_driver.h"
#include "eeprom_flash.h"


#define MQ135_ANALOG_PIN  PA0        // PA_0 as MQ-125 analog sensor for CO2
#define LED1              LED_BUILTIN
#define HEATER            PA7
#define WATER             PA6
#define WARNING_LED       PA5
#define DANGER_LED        PA4
#define BUTTON            PA3
// LCD 2004 i2c PINS:
// i2c SDA  - B7
// i2c SCL  - B6

// DHT11 CONNECTION
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int   pinDHT11 =           PA2;  // PA_2 as HDT11 sensor for TEMP and Humidity


SimpleDHT11 dht11(pinDHT11);

Value_stack CO2_PPM_stack;

On_off_driver heater(10);
On_off_driver water(5);

byte temperature = 0;
byte humidity = 0;

typedef struct {
    byte nothing;
    byte mode;
    byte hum;
    byte temp;
} flash_word;

flash_word config;


//flash_word *p_start_config;
#if LOCATION==LIVING_ROOM
  // FOR RED ONE - big - hall
  byte target_temp = 22;
  byte target_humidity = 49;
  byte comfort_temp = 20;
#else
  // FOR GREEN ONE - small - bedroom and YELLOW bedroom
  byte target_temp = 22;
  byte target_humidity = 54;
  byte comfort_temp = 20;
#endif

byte monitor_mode = 0;
int pass_adc_reading_cycles = 30;
int err = SimpleDHTErrSuccess;
int sensorValue = 0;


#if LOCATION==LIVING_ROOM
  LiquidCrystal_I2C  screen1(0x3F,2,1,0,4,5,6,7);
  // 0x3F is the I2C bus address  RED - BIG - living room
#else
  LiquidCrystal_I2C  screen1(0x27,2,1,0,4,5,6,7);
  // 0x27 is the I2C bus address  GREEN - SMALL - bedroom
#endif


int MHZ19B_ao_from_adc_to_ppm(int ADC_value){

  int MHZ19B_range = 5000;

  #if LOCATION==LIVING_ROOM
    int ppm_correction = -100;  //correction value for RED - BIG - hall
  #elif LOCATION==YELLOW_BEDROOM
    int ppm_correction = 50;  //correction value for YELLOW - bedroom
  #else
    int ppm_correction = -50;  //correction value for GREEN - SMALL - bedroom
  #endif

  //float Up = 5.0;
  float Uadc_max = 3.3;
  int ADC_steps = 1023;
  float Uadc = Uadc_max/ADC_steps*ADC_value;

  return (Uadc-0.4)/1.6*MHZ19B_range + ppm_correction;
};


int MQ135_ao_from_adc_to_ppm(int ADC_value, int temp_value, int humidity_value) {


  // FOR GREEN ONE
  #define RLOAD 1000
  /// Calibration resistance at atmospheric CO2 level
  #define RZERO 28000
  /// Parameters for calculating ppm of CO2 from sensor resistance
  #define PARA 116
  #define PARB 2.48828

  /*
  // FOR RED ONE
  #define RLOAD 1000
  /// Calibration resistance at atmospheric CO2 level
  #define RZERO 55000
  /// Parameters for calculating ppm of CO2 from sensor resistance
  #define PARA 116
  #define PARB 2.35828
  */

    float Up = 5.0;
    float Uadc_max = 3.3;
    int ADC_steps = 1023;

    float Uadc = Uadc_max/ADC_steps*ADC_value;

    /// Parameters to model temperature and humidity dependence
    #define CORA 0.0003
    #define CORB 0.02718
    #define CORC 1.39538
    #define CORD 0.0018
    float correction_factor = CORA * temp_value * temp_value - CORB * temp_value + CORC - (humidity_value-33.)*CORD;

    float resistance = ((Up/Uadc)*RLOAD - RLOAD)/correction_factor;

    return PARA * pow((resistance/RZERO), -PARB);
};

uint32_t t = 0;

void setup() {

    // Check stored config in flash

    t = readEEPROMWord(0);
    config = *((flash_word*)&t);
    //=====================================================================
    config.temp = target_temp;
    config.hum = target_humidity;
    //=====================================================================
    // for device with target temp and target humidity control by buttons
    //=====================================================================
    //if (config.temp < 15 || config.temp > 25 ) config.temp = target_temp;
    //if (config.hum < 30 || config.hum > 80 ) config.hum = target_humidity;

    if (config.mode > 1 ) config.mode = 1;
    config.nothing = 255;
    t = *((uint32_t*)&config);
    // Write default config if data is corrupted
    if (t!=readEEPROMWord(0)) {
      enableEEPROMWriting();
      writeEEPROMWord(0,t);
      disableEEPROMWriting();
    };

    // Set global parameters
    target_temp = config.temp;
    target_humidity = config.hum;
    comfort_temp = config.temp - 1;
    monitor_mode = config.mode;


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


    screen1.home (); // set cursor to 0,0
    screen1.print("Temp:----   Hum:----");
    screen1.setCursor(0,1);
    screen1.print(" CO2 level: wait...");
    //screen1.print(" CO2: ---");
    screen1.setCursor(0,2);
    screen1.print(" System is starting ");
    screen1.setCursor(0,3);
    if (monitor_mode == 0) {
      screen1.print("Heater:--- Water:---");
    } else {
      screen1.print("----monitor mode----");
    };

    delay(400);

    digitalWrite(WARNING_LED, LOW);
    digitalWrite(DANGER_LED, HIGH);

    delay(350);

    digitalWrite(WARNING_LED, LOW);
    digitalWrite(DANGER_LED, LOW);

    screen1.setBacklight(HIGH);

}

void loop() {

        delay(750);
        digitalWrite(LED1, HIGH);


        // check button state
        if ( digitalRead(BUTTON) == HIGH ) {
          // change current working mode
          if (monitor_mode == 0) {
            monitor_mode = 1;
            config.mode = 1;
            screen1.setCursor(0,3);
            screen1.print("----monitor mode----");
            //heater.stop();
            //water.stop();
          } else {
            monitor_mode = 0;
            config.mode = 0;
            screen1.setCursor(0,3);
            screen1.print("Heater:--- Water:---");
          };
        };

        // Check if config was changhed by user
        t = readEEPROMWord(0);
        if ( *((uint32_t*)&config) != t ) {
          enableEEPROMWriting();
          writeEEPROMWord(0, *((uint32_t*)&config));
          disableEEPROMWriting();
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
          //CO2_PPM_stack.add_value(MQ135_ao_from_adc_to_ppm(analog_value, temperature, humidity));
          CO2_PPM_stack.add_value(MHZ19B_ao_from_adc_to_ppm(analog_value));
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
          if (sensorValue > 800 && sensorValue <= 1400) {
            digitalWrite(WARNING_LED, HIGH);
            digitalWrite(DANGER_LED, LOW);
              screen1.setCursor(0,2);
              screen1.print("  Need ventilation! ");
          } else if (sensorValue > 1400){
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
