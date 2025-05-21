#include <Wire.h>
#include <Adafruit_MPR121.h>
#include "HID-Project.h"

// MPR121 I2C 地址
#define MPR121_ADDR_A 0x5A
#define MPR121_ADDR_B 0x5B
#define MPR121_ADDR_C 0x5C

// KB874 线路已经装在4号引脚
#define IR_PIN 4

// 按钮已经装在7、8、9号引脚
#define BUTTON_PIN_1 7
#define BUTTON_PIN_2 8
#define BUTTON_PIN_3 9

// LED 指示灯
#define LED_PIN 10

// 创建 MPR121 实例
Adafruit_MPR121 mprA = Adafruit_MPR121();
Adafruit_MPR121 mprB = Adafruit_MPR121();
Adafruit_MPR121 mprC = Adafruit_MPR121();

// 定义触摸区域键值（A-P）
char keyMap[3][6] = {
  { 'A', 'B', 'C', 'D', 'E', 0 },    // MPR A 对应 A-E
  { 'F', 'G', 'H', 'I', 'J', 'K' },  // MPR B 对应 F-K
  { 'L', 'M', 'N', 'O', 'P', 0 }     // MPR C 对应 L-P
};

// 初始化变量
unsigned long lastPollTime = 0;
const unsigned long pollInterval = 16;  // ms 每次轮调间隔
const int touchThreshold = 2;           // 触摸阀值，数值可调整

void setup() {
  // 初始化 LED 引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // 默认常亮（失败状态）

  // 初始化串口（仅用于调试，可删去）
  Serial.begin(9600);

  // 调用 MPR121 模块
  bool mprInitSuccess = true;
  if (!mprA.begin(MPR121_ADDR_A)) {
    mprInitSuccess = false;
  }
  if (!mprB.begin(MPR121_ADDR_B)) {
    mprInitSuccess = false;
  }
  if (!mprC.begin(MPR121_ADDR_C)) {
    mprInitSuccess = false;
  }

  // 设置 IR_PIN 为输入
  pinMode(IR_PIN, INPUT_PULLUP);

  // 设置按钮引脚为输入
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);

  // 初始化 NKRO键盘
  NKROKeyboard.begin();

  // 如果所有模块初始化成功，关闭 LED
  if (mprInitSuccess) {
    digitalWrite(LED_PIN, LOW);  // LED 关闭（正常状态）
  } else {
    // 如果有失败，保持 LED 常亮并停留在此
    while (1)
      ;
  }
}

void loop() {
  // 每48ms轮调一次
  if (millis() - lastPollTime >= pollInterval) {
    lastPollTime = millis();

    // 获取 MPR121 模块的触摸状态
    uint16_t touchedA = mprA.touched();
    uint16_t touchedB = mprB.touched();
    uint16_t touchedC = mprC.touched();

    // 检测 MPR A (1-5)
    for (int i = 0; i < 5; i++) {
      if ((touchedA >> (i + 1)) & 0x01 && mprA.filteredData(i + 1) > touchThreshold) {
        NKROKeyboard.add(keyMap[0][i]);
      } else {
        NKROKeyboard.release(keyMap[0][i]);
      }
    }

    // 检测 MPR B (0-5)
    for (int i = 0; i < 6; i++) {
      if ((touchedB >> i) & 0x01 && mprB.filteredData(i) > touchThreshold) {
        NKROKeyboard.add(keyMap[1][i]);
      } else {
        NKROKeyboard.release(keyMap[1][i]);
      }
    }

    // 检测 MPR C (0-4)
    for (int i = 0; i < 5; i++) {
      if ((touchedC >> i) & 0x01 && mprC.filteredData(i) > touchThreshold) {
        NKROKeyboard.add(keyMap[2][i]);
      } else {
        NKROKeyboard.release(keyMap[2][i]);
      }
    }

    // 检测 IR 接收线被遮挡情况
    if (digitalRead(IR_PIN) == HIGH) {  // 遮挡时输入空格
      NKROKeyboard.add(' ');
    } else {
      NKROKeyboard.release(' ');
    }

    // 检测按钮情况
    if (digitalRead(BUTTON_PIN_1) == LOW) {  // 按钮7被按下时输入3
      NKROKeyboard.add('3');
    } else {
      NKROKeyboard.release('3');
    }

    if (digitalRead(BUTTON_PIN_2) == LOW) {  // 按钮8被按下时输入2
      NKROKeyboard.add('2');
    } else {
      NKROKeyboard.release('2');
    }

    if (digitalRead(BUTTON_PIN_3) == LOW) {  // 按钮9被按下时输入1
      NKROKeyboard.add('1');
    } else {
      NKROKeyboard.release('1');
    }

    // 发送键盘事件
    NKROKeyboard.send();
  }
}
