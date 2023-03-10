#include <driver/adc.h>
#include <esp_bt.h>
#include <BleKeyboard.h>
#include <WiFi.h>


#define PIN_ROW_1 27
#define PIN_ROW_2 14
#define PIN_ROW_3 12
#define PIN_ROW_4 13
#define PIN_ROW_5 4
#define PIN_ROW_6 0
#define PIN_ROW_7 2
#define PIN_ROW_8 15

#define PIN_COL_1 18
#define PIN_COL_2 25
#define PIN_COL_3 33
#define PIN_COL_4 32
#define PIN_COL_5 19
#define PIN_COL_6 26

#define MATRIX_SIZE 38



BleKeyboard bleKeyboard("ble_left");



const byte keycode[MATRIX_SIZE] = {194, 195, 196, 197, 198, 199,         // F1  F2  F3  F4  F5  F6 
                                   177, 49, 50, 51, 52, 53,              // ESC  1  2  3  4  5
                                   179, 113, 119, 101, 114, 116,              // Tab  q  w  e  r  t
                                   129, 97, 115, 100, 102, 103,              // Left_Shift  a  s  d  f  g 
                                   128, 122, 120, 99, 118, 98,              // Left_Control  z  x  c  v  b
                                   91, 93, 32, 178,                      // Alt  Right_Ctrl  Key_Reload  Function_Key
                                   131, 130,                             // window  Left_alt
                                   193, 237};                            // CapsLock  Menu

//const byte keycode[MATRIX_SIZE] = {65, 65, 65, 65, 65, 65,         // F1  F2  F3  F4  F5  F6 
//                                   65, 65, 65, 65, 65, 65,              // ESC  1  2  3  4  5
//                                   65, 65, 65, 65, 65, 65,              // Caps_Lock  a  s  d  f  g
//                                   65, 65, 65, 65, 65, 65,              // Left_Shift  z  x  c  v  b 
//                                   65, 65, 65, 65, 65, 65,              // Left_Control  Win  `  -  +  Right_Alt
//                                   65, 65, 65, 65,                      // Alt  Right_Ctrl  Key_Reload  Function_Key
//                                   65, 65,                             // window  Left_alt
//                                   65, 65};                            // Enter  Menu




//const byte keycode[MATRIX_SIZE] = {27, 49, 50, 51, 52, 53,              // Esc  1  2  3  4  5 
//                                   179, 113, 119, 101, 114, 116,        // Tab  q  w  e  r  t
//                                   193, 97, 115, 100, 102, 103,         // Caps_Lock  a  s  d  f  g
//                                   129, 122, 120, 99, 118, 98,          // Left_Shift  z  x  c  v  b 
//                                   128, 131, 96, 45, 43, 134,           // Left_Control  Win  `  -  +  Right_Alt
//                                   134, 132, 0, 0,                      // Alt  Right_Ctrl  Key_Reload  Function_Key
//                                   32, 237,                             // Space  Menu
//                                   0, 0};                               // Left_click  Right_click
                                   
//const byte fnkeycode[MATRIX_SIZE] = {194, 195, 196, 197, 198, 199,      // F1  F2  F3  F4  F5  F6
//                                     179, 113, 119, 101, 114, 116,      // Tab  q  w  e  r  t
//                                     193, 97, 115, 100, 102, 103,       // Caps_Lock  a  s  d  f  g
//                                     129, 122, 120, 99, 118, 98,        // Left_Shift  z  x  c  v  b 
//                                     128, 131, 96, 45, 43, 134,         // Left_Control  Win  `  -  +  Right_Alt
//                                     134, 132, 0, 0,                    // Alt  Right_Ctrl  Key_Reload  Function_Key
//                                     32, 237,                           // Space  Menu
//                                     0, 0};                             // Left_click  Right_click


// ????????? ?????? ????????? ????????? ??????
bool bufferA[MATRIX_SIZE];
bool bufferB[MATRIX_SIZE];

// ?????? ????????? ??????/?????? ????????? ??????????????? ????????????
bool *prevBuffer = &bufferA[0];
bool *curBuffer = &bufferB[0];
bool *tempBuffer;

// ????????? ?????? ????????? ??????????????? ????????????
#define DEBOUNCE_DELAY 50


// ????????? ??????????????? ??????????????? ????????? ????????? ????????? ??????
unsigned long lastChange[MATRIX_SIZE];
unsigned long curMillis;

//deep sleep??? ????????? ????????? ??????
unsigned long sleepCounter = 0;

void setup()
{
  

  // ?????? ??????(???????????? ??????, ???????????? ??????)
  WiFi.disconnect(true);  // Disconnect from the network
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
  adc_power_off();
  Serial.begin(115200);
  bleKeyboard.begin();


  // ?????? ????????? ????????? ????????? ??????????????? (1: ????????????, 0: ??? ?????????)
  for (int i = 0; i < MATRIX_SIZE; i++) {
    bufferA[i] = 1;
    bufferB[i] = 1;
  }
  

  pinMode(PIN_ROW_1, OUTPUT);
  pinMode(PIN_ROW_2, OUTPUT);
  pinMode(PIN_ROW_3, OUTPUT);
  pinMode(PIN_ROW_4, OUTPUT);
  pinMode(PIN_ROW_5, OUTPUT);
  pinMode(PIN_ROW_6, OUTPUT);
  pinMode(PIN_ROW_7, OUTPUT);
  pinMode(PIN_ROW_8, OUTPUT);

  digitalWrite(PIN_ROW_1, HIGH);
  digitalWrite(PIN_ROW_2, HIGH);
  digitalWrite(PIN_ROW_3, HIGH);
  digitalWrite(PIN_ROW_4, HIGH);
  digitalWrite(PIN_ROW_5, HIGH);
  digitalWrite(PIN_ROW_6, HIGH);
  digitalWrite(PIN_ROW_7, HIGH);
  digitalWrite(PIN_ROW_8, HIGH);

  pinMode(PIN_COL_1, INPUT_PULLUP);
  pinMode(PIN_COL_2, INPUT_PULLUP);
  pinMode(PIN_COL_3, INPUT_PULLUP);
  pinMode(PIN_COL_4, INPUT_PULLUP);
  pinMode(PIN_COL_5, INPUT_PULLUP);
  pinMode(PIN_COL_6, INPUT_PULLUP);



  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  touchAttachInterrupt(T7,callback, 40);
  esp_sleep_enable_touchpad_wakeup();

  
}

void loop()
{
  while (bleKeyboard.isConnected())
  {
    // ??????????????? ??????????????? ???????????? ????????? ????????? ????????????
    digitalWrite(PIN_ROW_1, LOW);
    curBuffer[0] = digitalRead(PIN_COL_1);
    curBuffer[1] = digitalRead(PIN_COL_2);
    curBuffer[2] = digitalRead(PIN_COL_3);
    curBuffer[3] = digitalRead(PIN_COL_4);
    curBuffer[4] = digitalRead(PIN_COL_5);
    curBuffer[5] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_1, HIGH);
    digitalWrite(PIN_ROW_2, LOW);
    curBuffer[6] = digitalRead(PIN_COL_1);
    curBuffer[7] = digitalRead(PIN_COL_2);
    curBuffer[8] = digitalRead(PIN_COL_3);
    curBuffer[9] = digitalRead(PIN_COL_4);
    curBuffer[10] = digitalRead(PIN_COL_5);
    curBuffer[11] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_2, HIGH);
    digitalWrite(PIN_ROW_3, LOW);
    curBuffer[12] = digitalRead(PIN_COL_1);
    curBuffer[13] = digitalRead(PIN_COL_2);
    curBuffer[14] = digitalRead(PIN_COL_3);
    curBuffer[15] = digitalRead(PIN_COL_4);
    curBuffer[16] = digitalRead(PIN_COL_5);
    curBuffer[17] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_3, HIGH);
    digitalWrite(PIN_ROW_4, LOW);
    curBuffer[18] = digitalRead(PIN_COL_1);
    curBuffer[19] = digitalRead(PIN_COL_2);
    curBuffer[20] = digitalRead(PIN_COL_3);
    curBuffer[21] = digitalRead(PIN_COL_4);
    curBuffer[22] = digitalRead(PIN_COL_5);
    curBuffer[23] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_4, HIGH);
    digitalWrite(PIN_ROW_5, LOW);
    curBuffer[24] = digitalRead(PIN_COL_1);
    curBuffer[25] = digitalRead(PIN_COL_2);
    curBuffer[26] = digitalRead(PIN_COL_3);
    curBuffer[27] = digitalRead(PIN_COL_4);
    curBuffer[28] = digitalRead(PIN_COL_5);
    curBuffer[29] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_5, HIGH);
    digitalWrite(PIN_ROW_6, LOW);
    curBuffer[30] = digitalRead(PIN_COL_3);
    curBuffer[31] = digitalRead(PIN_COL_4);
    curBuffer[32] = digitalRead(PIN_COL_5);
    curBuffer[33] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_6, HIGH);
    digitalWrite(PIN_ROW_7, LOW);
    curBuffer[34] = digitalRead(PIN_COL_5);
    curBuffer[35] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_7, HIGH);
    digitalWrite(PIN_ROW_8, LOW);
    curBuffer[36] = digitalRead(PIN_COL_5);
    curBuffer[37] = digitalRead(PIN_COL_6);
    digitalWrite(PIN_ROW_8, HIGH);

    // ?????? ????????? ????????? ????????? ????????????
    curMillis = millis();

    // ?????? ?????? ????????? ?????? ??? deep sleep mode??? ????????????
    if(curMillis - sleepCounter > 300000){
            esp_deep_sleep_start();
            }

    // ??????/?????? ?????? ????????? ???????????? ?????? ?????? ????????????
    for (int i = 0; i < MATRIX_SIZE; i++) {
      
      // ????????? ????????? ????????? ?????? ?????? ?????? ?????? ????????????
      if (prevBuffer[i] == curBuffer[i]){
        continue;
        }

      // ?????? ?????? ???????????? ????????? ???????????? ??????????????? ???????????? ????????????
      if (curMillis - lastChange[i] < DEBOUNCE_DELAY) {
        curBuffer[i] = prevBuffer[i];
        continue;
      } else{

      // ????????? ????????? ?????? ?????? ????????? ???????????? ????????? ????????????
      lastChange[i] = curMillis;
      }
    
      // ?????? ??????????????? ???????????? PC??? ????????? ????????? ????????????
      if (curBuffer[i] == 0){
          bleKeyboard.press(keycode[i]);
          sleepCounter = millis();
        }
      else{
          bleKeyboard.release(keycode[i]);
        }
    }


    tempBuffer = prevBuffer;
    prevBuffer = curBuffer;
    curBuffer = tempBuffer;
  }
}

void callback(){
}
