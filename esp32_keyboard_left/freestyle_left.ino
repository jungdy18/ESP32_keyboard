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


// 스위치 스캔 결과를 기록할 배열
bool bufferA[MATRIX_SIZE];
bool bufferB[MATRIX_SIZE];

// 어떤 배열에 이전/현재 결과가 들어있는지 지정한다
bool *prevBuffer = &bufferA[0];
bool *curBuffer = &bufferB[0];
bool *tempBuffer;

// 채터링 현상 방지용 지연시간을 지정한다
#define DEBOUNCE_DELAY 50


// 키별로 마지막으로 상태변화가 일어난 시간을 기록할 배열
unsigned long lastChange[MATRIX_SIZE];
unsigned long curMillis;

//deep sleep에 들어갈 시간을 기록
unsigned long sleepCounter = 0;

void setup()
{
  

  // 기본 세팅(와이파이 끄기, 블루투스 켜기)
  WiFi.disconnect(true);  // Disconnect from the network
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
  adc_power_off();
  Serial.begin(115200);
  bleKeyboard.begin();


  // 스캔 결과를 기록할 배열을 초기화한다 (1: 대기상태, 0: 키 눌러짐)
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
    // 스위치별로 입력상태를 확인한고 결과를 버퍼에 기록한다
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

    // 현재 스캔이 완료된 시간을 기록한다
    curMillis = millis();

    // 일정 시간 반응이 없을 시 deep sleep mode에 들어간다
    if(curMillis - sleepCounter > 300000){
            esp_deep_sleep_start();
            }

    // 이전/현재 스캔 결과를 비교하여 모든 키를 처리한다
    for (int i = 0; i < MATRIX_SIZE; i++) {
      
      // 키입력 상태의 변화가 없을 때는 다음 키로 넘어간다
      if (prevBuffer[i] == curBuffer[i]){
        continue;
        }

      // 너무 짧은 시간안에 상태가 변했다면 채터링으로 간주하고 무시한다
      if (curMillis - lastChange[i] < DEBOUNCE_DELAY) {
        curBuffer[i] = prevBuffer[i];
        continue;
      } else{

      // 문제가 없다면 해당 키의 마지막 상태변화 시간을 갱신한다
      lastChange[i] = curMillis;
      }
    
      // 키를 변화방향을 확인하고 PC로 적절한 신호를 전송한다
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
