
/*
  Home Climatizer 1.5
*/

#include "location_definition.h"
#include "Wire.h"
#include "SimpleDHT.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include "Value_stack.h"
#include "On_off_driver.h"
#include "eeprom_flash.h"
#include "pin_definition.h"

#if LOCATION==LIVING_ROOM
  #define SELF_HEATING_TEMP_DELTA 1
#elif LOCATION==YELLOW_BEDROOM
  #define SELF_HEATING_TEMP_DELTA 4
#else
  // ====== BEDROOM
  #define SELF_HEATING_TEMP_DELTA 1
#endif

// ============================ setting of initial target values ==============
#if LOCATION==LIVING_ROOM
  // FOR RED ONE - big - hall
  byte target_temp = 22;
  byte target_humidity = 49;
  byte comfort_temp = 20;
#else
  // FOR GREEN ONE - small - bedroom and YELLOW bedroom
  byte target_temp = 22;
  byte target_humidity = 56;
  byte comfort_temp = 20;
#endif


// define target ranges
#define MIN_TARGET_TEMP_DELTA   -5
#define MAX_TARGET_TEMP_DELTA   5

#define MIN_TARGET_TEMP         10
#define MAX_TARGET_TEMP         25

#define MIN_TARGET_HUMIDITY     40
#define MAX_TARGET_HUMIDITY     80

#define HEATER_SWITCHING_DELAY  15
#define WATER_SWITCHING_DELAY   10

SimpleDHT11 dht11(DHT11_SENSOR_PIN);
Value_stack CO2_PPM_stack;
On_off_driver heater(HEATER_SWITCHING_DELAY);
On_off_driver water(WATER_SWITCHING_DELAY);

byte temperature = 0;
byte humidity = 0;

typedef struct {
    int8_t temp_delta;
    byte mode;
    byte hum;
    byte temp;
} flash_record;

flash_record  current_target_state;

union config_word {
  flash_record  record;
  uint32_t      in_buffer_format;
} config;

FlashBuffer memory;

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

//=============================================================================

int convert_ADC_to_PPM(int ADC_value){

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

void setup() {
  
  // get stored config from flash memory
  // memory.eraseMemory();

  config.in_buffer_format = memory.readWord();
  
  // check if values from flash memory storage are correct
  if (config.in_buffer_format == 0xFFFFFFFF) {
    current_target_state.temp_delta = SELF_HEATING_TEMP_DELTA;
    current_target_state.mode = 0;
    current_target_state.hum = target_humidity;
    current_target_state.temp = target_temp;
  } else {
    current_target_state = config.record;
    if (current_target_state.temp_delta > MAX_TARGET_TEMP_DELTA 
        || current_target_state.temp_delta < MIN_TARGET_TEMP_DELTA) current_target_state.temp_delta = SELF_HEATING_TEMP_DELTA;

    if (current_target_state.mode > 1 ) current_target_state.mode = 0;

    if (current_target_state.hum < MIN_TARGET_HUMIDITY 
        || current_target_state.hum > MAX_TARGET_HUMIDITY ) current_target_state.hum = target_humidity;

    if (current_target_state.temp < MIN_TARGET_TEMP 
        || current_target_state.temp > MAX_TARGET_TEMP ) current_target_state.temp = target_temp;
  }
  
  config.record = current_target_state;

  // Write default config if data is not equal
  if (config.in_buffer_format != memory.readWord()) {
    memory.writeWord(config.in_buffer_format);
  };

  // Set global parameters
  target_temp = current_target_state.temp;
  target_humidity = current_target_state.hum;
  comfort_temp = current_target_state.temp - 1; // Histeresis emulation
  monitor_mode = current_target_state.mode;

  // define pin modes
  pinMode(LED1, OUTPUT);
  pinMode(HEATER_CONTROL_PIN, OUTPUT);
  digitalWrite(HEATER_CONTROL_PIN, LOW);
  pinMode(WATER_CONTROL_PIN, OUTPUT);
  digitalWrite(WATER_CONTROL_PIN, LOW);
  pinMode(WARNING_LED, OUTPUT);
  digitalWrite(WARNING_LED, HIGH);
  pinMode(DANGER_LED, OUTPUT);
  digitalWrite(DANGER_LED, LOW);
  pinMode(CHANGE_MODE_BUTTON, INPUT);

  
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

  delay(1000);
  digitalWrite(LED1, HIGH);

  // check mode button state
  if ( digitalRead(CHANGE_MODE_BUTTON) == HIGH ) {
    // change current working mode
    if (monitor_mode == 0) {
      monitor_mode = 1;
      current_target_state.mode = 1;
      screen1.setCursor(0,3);
      screen1.print("----monitor mode----");
      //heater.stop();
      //water.stop();
    } else {
      monitor_mode = 0;
      current_target_state.mode = 0;
      screen1.setCursor(0,3);
      screen1.print("Heater:--- Water:---");
    };
  };
        
  // Check if config was changhed by user
  config.in_buffer_format = memory.readWord();
  if (current_target_state.temp_delta != config.record.temp_delta
      || current_target_state.mode != config.record.mode 
      || current_target_state.temp != config.record.temp
      || current_target_state.hum != config.record.hum ) {
        
    //save new config to flash memory
    config.record = current_target_state;
    memory.writeWord(config.in_buffer_format);
  };

  heater.tic_tac();
  water.tic_tac();

  // read current temp and hum and check DHT11 sensor error after it
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    temperature=0;
    humidity=0;
  } else {
    temperature = temperature - current_target_state.temp_delta; 
  };

  screen1.setCursor(5,0);
  screen1.print("+");
  screen1.print(temperature);
  screen1.print((char)223);
  screen1.print("C ");

  screen1.setCursor(16,0);
  screen1.print(humidity);
  screen1.print("%  ");


  // =========================== process data sensor data ===================
  if ( monitor_mode == 0) {
    screen1.setCursor(0,2);
    if (temperature == 0 && humidity == 0) {

      screen1.print(" DHT11 Sensor Error!");
      heater.set_state(ON);
      water.set_state(ON);

    } else if (temperature > target_temp && humidity > target_humidity) {

      screen1.print("  Comfort condition ");
      heater.set_state(OFF);
      water.set_state(OFF);

    } else if (temperature > target_temp && humidity <= target_humidity) {

      screen1.print("      Too dry!      ");
      heater.set_state(OFF);
      water.set_state(ON);

    } else if (temperature <= target_temp && humidity <= target_humidity){

      if (temperature >= comfort_temp) {
        screen1.print("      Too dry!      ");
      } else {
        screen1.print(" Too cold! Too dry! ");
      };
      heater.set_state(ON);
      water.set_state(ON);

    } else if (temperature <= target_temp && humidity > target_humidity){

      if (temperature >= comfort_temp) {
        screen1.print("  Normal condition  ");
      } else {
        screen1.print("     Too cold!      ");
      };
      heater.set_state(ON);
      water.set_state(OFF);

    } else {
      screen1.print("  Normal condition  ");
    };

    if(heater.get_state() == OFF) {
      digitalWrite(HEATER_CONTROL_PIN, LOW);
      screen1.setCursor(7,3);
      screen1.print("off");
    } else {
      digitalWrite(HEATER_CONTROL_PIN, HIGH);
      screen1.setCursor(7,3);
      screen1.print("ON ");
    };

    if(water.get_state() == OFF) {
      digitalWrite(WATER_CONTROL_PIN, LOW);
      screen1.setCursor(17,3);
      screen1.print("off");
    } else {
      digitalWrite(WATER_CONTROL_PIN, HIGH);
      screen1.setCursor(17,3);
      screen1.print("ON ");
    };
  } else {
    digitalWrite(HEATER_CONTROL_PIN, LOW);
    digitalWrite(WATER_CONTROL_PIN, LOW);
    screen1.setCursor(0,2);
    screen1.print("                    ");
  };
  //===========================================================================

  delay(1000);
  digitalWrite(LED1, LOW);

  if (pass_adc_reading_cycles == 0) {

      CO2_PPM_stack.add_value(convert_ADC_to_PPM((int)analogRead(ANALOG_SENSOR_PIN)));
      sensorValue = CO2_PPM_stack.get_average();

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


  // ===================== setting PPM alarm LEDS ============================
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
  // ===========================================================================
};
