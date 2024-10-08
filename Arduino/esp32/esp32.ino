#include <HX711.h>
#include "SoftwareSerial.h"

// 핀 설정
#define buzz 27
#define DOUT 25  //엠프 데이터 아웃 핀 넘버 선언
#define CLK 26
#define BATTERY_PIN 34  // 배터리 전압을 측정할 핀 (ADC 포트)

// esp32 동작에 필요한 전역 변수들
float calibration_factor = 4900;  //캘리브레이션 값
float bat_max;
float bat_min;
float voltage;  // 배터리 전압
int sensorValue;
int battery;        // 배터리 퍼센트
double weight = 0;  // 측정된 무게 값

String password = "qwerty1234";  // 초기 비밀번호
boolean auth = false;            // 인증 여부

// 블루투스 설정
SoftwareSerial bluetooth(16, 17);

// 무게 센서 설정
HX711 scale(DOUT, CLK);  //엠프 핀 선언

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  pinMode(buzz, OUTPUT);

  scale.set_scale();                    //무게 측정용 초기화
  scale.tare();                         //Reset the scale to 0
  scale.set_scale(calibration_factor);  //스케일 지정

  pinMode(BATTERY_PIN, INPUT);
}

void loop() {
  sensorValue = analogRead(BATTERY_PIN);  // 아날로그 값 읽기 (ADC 34번 핀)
  Serial.println(sensorValue);

  bat_max = 4096 * (2.1 / 3.3);
  bat_min = 4096 * (1.6 / 3.3);
  battery = (int)((sensorValue - bat_min) / (bat_max - bat_min) * 100);
  voltage = ((battery * 0.005) + 1.6) * 2;  // 측정된 전압에 곱하기 2를 하여 실제 전압 계산

  if (bluetooth.available() > 0 && !auth) {             // 블루투스 사용 가능하면 (인증전)
    String readData = bluetooth.readStringUntil('\n');  // 개행 문자까지 읽음

    Serial.println(readData);

    if (readData == " " || readData.startsWith("CONNECT")) {
      Serial.println("데이터 무시");
    } else {
      if (readData == password) {
        auth = true;
        Serial.println("인증 성공!!!");
        bluetooth.println("auth_suc");
      } else {
        auth = false;
        Serial.println("인증 실패!!!");
        bluetooth.println("auth_fail");
      }
    }
    delay(200);
  }
  
  if (bluetooth.available() > 0 && auth) {              // 블루투스 사용 가능하면 (인증후)
    String readData = bluetooth.readStringUntil('\n');  // 개행 문자까지 읽음

    if (readData.startsWith("DISCONNECT")) {
      Serial.println("인증 취소!!!");
      auth = false;
    }

    if (auth) {
      if (readData == "menu 1") {  // 벨 울리기
        bluetooth.println("ring_suc");
        delay(200);
        while (1) {
          digitalWrite(27, HIGH);
          delay(500);
          digitalWrite(27, LOW);
          delay(500);
          Serial.println("벨울림");

          readData = bluetooth.readStringUntil('\n');
          if (readData == "menu 2") {  // 벨 울리기 중지
            bluetooth.println("ring_stop");
            Serial.println("벨꺼짐");
            break;
          }
        }
        delay(200);
      }

      if (readData == "menu 3") {             // 무게 값을 전송
        scale.set_scale(calibration_factor);  //캘리브레이션 값 적용
        Serial.print("Reading: ");
        Serial.print(scale.get_units(), 1);  //무게 출력
        Serial.print(" kg");                 //단위
        Serial.println();
        weight = scale.get_units();

        Serial.println(String(weight));
        bluetooth.println(String(weight));
        delay(200);
      }

      if (readData == "menu 4") {  // 배터리 상태 전송
        Serial.println(String(battery) + "/" + String(voltage));
        bluetooth.println(String(battery) + "/" + String(voltage));
      }
    }
  }

  delay(500);
}
