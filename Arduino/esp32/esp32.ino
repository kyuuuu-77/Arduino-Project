#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include "HX711.h"


#define buzz 27
#define DOUT 25  //엠프 데이터 아웃 핀 넘버 선언
#define CLK 26

#define BATTERY_PIN 34  // 배터리 전압을 측정할 핀 (ADC 입력 핀)

#define SCREEN_WIDTH 128  // OLED 디스플레이의 가로 해상도
#define SCREEN_HEIGHT 32  // OLED 디스플레이의 세로 해상도

#define OLED_RESET -1  // 리셋 핀이 필요하지 않으면 -1로 설정
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float calibration_factor = 4900;  //캘리브레이션 값
HX711 scale(DOUT, CLK);           //엠프 핀 선언
int battery = 0;
double weight = 0;
double array[2];
int i = 0;


String line2;
String line3;
String line4;

SoftwareSerial bluetooth(16, 17);

int cnt = 0;

void setup() {
  // (SDA: GPIO 33, SCK: GPIO 32)
  Wire.begin(33, 32);

  Serial.begin(9600);
  bluetooth.begin(9600);

  pinMode(buzz, OUTPUT);

  scale.set_scale();                    //무게 측정용 초기화
  scale.tare();                         //Reset the scale to 0
  scale.set_scale(calibration_factor);  //스케일 지정
  Serial.println("ready");




  // 디스플레이 초기화
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // I2C 주소는 보통 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // 초기화 실패 시 멈춤
  }

  pinMode(BATTERY_PIN, INPUT);

  line3 = "NULL";
  line4 = "NULL";
}


// 9.22 16:08 갱신 -> 최대 충전전압 : 3.79, 최대 전압 : 3.68 or 3.67
void loop() {
  // 디스플레이에 텍스트 출력
  int sensorValue = analogRead(BATTERY_PIN);         // 아날로그 값 읽기 (ADC 34번 핀)
  float voltage = sensorValue * (3.3 / 4095.0) * 2;  // 측정된 전압에 곱하기 2를 하여 실제 전압 계산

  display.clearDisplay();
  display.setTextSize(1);                 // 텍스트 크기
  display.setTextColor(SSD1306_WHITE);    // 텍스트 색상
  display.setCursor(0, 0);                // 시작 위치 설정
  display.println(F("Battery Voltage"));  // 걍 텍스트
  line2 = String(voltage) + " V";
  display.println(line2);  // 전압 출력

  if (bluetooth.available() > 0) {
    String readData = bluetooth.readStringUntil('\n');  // 개행 문자까지 읽음
    line3 = readData;

    if (readData == "menu 1") {  // 벨을 울리기 시작
      line4 = "ring_suc";
      bluetooth.println("ring_suc");
      delay(200);
      while (1)  // 모드 2: 부저 울리기
      {

        digitalWrite(27, HIGH);
        delay(500);

        digitalWrite(27, LOW);
        delay(500);
        String mode = Serial.readStringUntil('\n');

        if (readData == "menu 2") {

          break;
        }
      }
      // 벨 울리기 중지
      line4 = "ring_stop";
      bluetooth.println("ring_stop");
      delay(200);
    }

    if (readData == "menu 3") {             // 무게 값을 전송
      scale.set_scale(calibration_factor);  //캘리브레이션 값 적용
      Serial.print("Reading: ");
      Serial.print(scale.get_units(), 1);  //무게 출력
      Serial.print(" kg");                 //단위
      Serial.println();
      weight = scale.get_units();
      for (i = 0; i < 3; i++)
        ;
      {
        weight = scale.get_units();

        if (weight < 0.1 || weight > array[i] + 5) {
          Serial.println("무게측정 오류입니다.");
          array[i] = 0;
          i = i - 1;
        }

        else {
          array[i] = weight;
        }
      }
      double kg = array[0] + array[1] + array[2];
      kg = kg / 3;
      i = 0;



      line4 = String(weight) + " Kg";
      bluetooth.println(String(weight));
      weight += 1.0;
      delay(200);
    }
    if (readData == "menu 4") {  // 배터리 상태를 전송
      if (battery > 2) {
        battery = 0;
      }
      line4 = "Bat Status : " + String(battery);
      bluetooth.println(String(battery));
      battery += 1;
      delay(200);
    }
  }

  display.println(line3);
  display.println(line4);
  display.display();  // 화면에 출력 (업데이트)

  delay(500);
}
