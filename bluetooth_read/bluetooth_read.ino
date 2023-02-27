#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define MATRIX_SIZE 38

byte keycode[MATRIX_SIZE] = {27, 49, 50, 51, 52, 53,              // Esc  1  2  3  4  5 
                                   97, 113, 97, 113, 97, 113,        // Tab  q  w  e  r  t
                                   97, 113, 97, 113, 97, 113,         // Caps_Lock  a  s  d  f  g
                                   97, 113, 97, 113, 97, 113,          // Left_Shift  z  x  c  v  b 
                                   97, 113, 97, 113, 97, 113,           // Left_Control  Win  `  -  +  Right_Alt
                                   97, 113, 97, 113,                      // Alt  Right_Ctrl  Key_Reload  Function_Key
                                   97, 113,                             // Space  Menu
                                   97, 113};                               // Left_click  Right_click


// ble 클래스
BLEServer* pServer = NULL;
// 통신용 캐릭터리스틱 클래스
BLECharacteristic* pCharacteristic = NULL;
// 연결 상태 확인용
bool deviceConnected = false;
// 연결 상태가 변경된 것을 확인하기 위한 변수
bool oldDeviceConnected = false;
// 앱이나 Central(Client) 기기와 연결되면 이 변수 값을 주기적으로 전송한다.
// 해당 값은 전송할때마다 1씩 증가한다.
uint32_t value = 0;

// BLE 서비스 UUID 
#define SERVICE_UUID        "a81fe6a0-cfcc-4fea-bc57-a160f17796ce"
// 캐릭터리스틱 UUID
#define CHARACTERISTIC_UUID "0ee8fae1-da42-4e31-a374-77a606ed20a1"

// 상태 변경 콜백함수
class MyServerCallbacks: public BLEServerCallbacks {
    // 연결 되면 호출
    void onConnect(BLEServer* pServer) {      
      deviceConnected = true;
    };
    // 연결이 끊어지면 호출
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  // 시리얼 모니터 시작
  Serial.begin(115200);

  // BLE 장치 초기화(장치명은 ESP32)
  BLEDevice::init("ESP32");

  // ESP32 BLE 장치를 서버(Peripheral) 기기로 생성
  pServer = BLEDevice::createServer();
  // 콜백 함수 등록 여기선 연결/연결 해제만 사용
  pServer->setCallbacks(new MyServerCallbacks());

  // 서비스 UUID 생성
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 캐릭터리스틱 생성
  // 해당 캐릭터리스틱엔 다음 속성들이 있다.
  // 읽기(read), 쓰기(write), 보내기(notify), 보내고 응답확인(indicate)  
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pCharacteristic->setValue(keycode, 38);
  // 캐릭터리스틱에 디스크립터 등록 2902 uuid가 등록되어야 notify와 indicate를 쓸수 있음  
  pCharacteristic->addDescriptor(new BLE2902());

  // 서비스 시작!
  pService->start();

  // 어드버타이징 클래스 얻기
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  // 어드버타이징에 서비스 UUID 추가
  pAdvertising->addServiceUUID(SERVICE_UUID);
  // scan response 사용 안함(어드버타이징 후 추가 데이터 전송)
  pAdvertising->setScanResponse(false);
  // 커넥션 인터벌 설정 - 0x00은 사용 안함
  pAdvertising->setMinPreferred(0x0);
  // 어드버아티징 시작
  BLEDevice::startAdvertising();
  // 디버그 메시지 출력
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // 장치가 연결되었다면 
    if (deviceConnected) {        
        // 값을 value에 설정 이때 설정된 값은 앱이나 Central기기에서 read 명령으로 읽을 수 있다.

        // value에 설정된 값을 연결된 기기에 전송
        pCharacteristic->notify();
        // value 값을 1 증가
        value++;
        // 3ms 지연
        //delay(3);
        // 원래는 3ms이지만 테스트 편의상 300ms로 변경하였다.
        delay(300); 
    }
    // 연결이 끊어질때
    // 현재 연결상태가 연결 해제되었고 이전 연결상태는 연결된 상태라면
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); //500ms 지연 블루투스 스택이 잘 정리될 시간을 줌 
        pServer->startAdvertising(); // 다시 어드버타이징 시작
        // 디버그 메시지 출력
        Serial.println("start advertising");
        // 이전 상태값을 현재 값으로 바꿈
        oldDeviceConnected = deviceConnected;
    }
    // 연결중
    // 현재 연결상태는 연결된 상태이고 이전 연결상태는 연결이 해제된 상태라면
    if (deviceConnected && !oldDeviceConnected) {
        // 이전 상태값을 현재 값으로 바꿈
        oldDeviceConnected = deviceConnected;
    }
}
