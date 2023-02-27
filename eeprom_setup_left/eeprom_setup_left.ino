#include <EEPROM.h>
#include <driver/adc.h>
#include <esp_bt.h>
#include <BleKeyboard.h>
#include <BleMouse.h>
#include <WiFi.h>


#define MATRIX_SIZE 38
#define EEPROM_SIZE 76

byte key_code[MATRIX_SIZE];

byte fnkey_code[MATRIX_SIZE];

BleKeyboard bleKeyboard("ble_left");
BleMouse bleMouse;

void setup() {


  // 기본 세팅(와이파이 끄기, 블루투스 켜기)
  WiFi.disconnect(true);  // Disconnect from the network
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
  adc_power_off();
  Serial.begin(115200);
  bleKeyboard.begin();

  
  // put your setup code here, to run once:
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

int location = 0;

EEPROM.get(location, key_code);

location+=sizeof(key_code);

EEPROM.get(location, fnkey_code);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
//  for(int i = 0; i < sizeof(key_code); i++) {               // EEPROM에 데이터 읽기
//    Serial.println(key_code[i]);
//    delay(1000);
//    Serial.println(fnkey_code[i]);
//    delay(1000);
//  }

}
