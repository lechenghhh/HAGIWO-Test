#include <stdlib.h>
#include <stdio.h>

const int KNOB_PIN = 1;
const int BUTTON_PIN = 7;
const int CLK_IN = 2;
const int CV_IN = 0;
const int PIN_NUM = 6;

const int CV_PIN[PIN_NUM] = { 3, 5, 6, 9, 10, 11 };
float cv[PIN_NUM] = { 0, 0, 0, 0, 0, 0 };
float coef[PIN_NUM] = { 1, 0.73, 0.57, 0.41, 0.29, 0.13 };
double wave_position[PIN_NUM] = {};
double lfo_length = 16384;
int lfo_type = 0;

int abs_knob = 2048;

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(CLK_IN, INPUT);
  for (int i = 0; i < PIN_NUM; i++)
    pinMode(CV_PIN[i], OUTPUT);

  abs_knob = analogRead(KNOB_PIN);
  showKnobValue();
  delay(3000);
}

int tmp_long_btn = 0;
bool tmp_btn = 0;
bool tmp_clk = 0;

void loop() {
  //read btn
  if (digitalRead(BUTTON_PIN) == 0 && tmp_btn == 1) {
    tmp_btn = 0;
    genCoefValue();  //生成随机变化
  }
  //long btn
  if (!digitalRead(BUTTON_PIN)) {
    tmp_long_btn++;
  }
  if (digitalRead(BUTTON_PIN) == 1 && tmp_btn == 0) {
    tmp_btn = 1;
    if (tmp_long_btn > 100) {  //长按事件响应
      lfo_type++;              //lfo type change
      if (lfo_type > 7) lfo_type = 0;
    }
    tmp_long_btn = 0;
  }

  //read clk
  if (digitalRead(CLK_IN) == 1 && tmp_clk == 0) {
    tmp_clk = 1;
    for (int i = 0; i < PIN_NUM; i++) wave_position[i] = 0;
  }
  if (digitalRead(CLK_IN) == 0 && tmp_clk == 1) {
    tmp_clk = 0;
  }

  //根据旋钮、cv来设置频率与波形
  float knob_coef = (4096 - analogRead(KNOB_PIN)) + (analogRead(CV_IN));  //获取旋钮值与压控
  for (int i = 0; i < PIN_NUM; i++) {
    if (wave_position[i] > lfo_length) wave_position[i] = 0;  //2^13
    wave_position[i] += knob_coef * coef[i] / 2;
    // wave select
    switch (lfo_type) {
      default:  //Sin
        if (wave_position[i] < lfo_length / 2)
          cv[i] = (2 * wave_position[i] / lfo_length) * 255;
        else
          cv[i] = 2 * (1 - (wave_position[i] / lfo_length)) * 255;
        break;
      case 1:  //Pluse 1
        if (wave_position[i] < lfo_length / 4)
          cv[i] = 255;
        else
          cv[i] = 0;
        break;
      case 2:  //PLUSE 2
        if (wave_position[i] < lfo_length / 2)
          cv[i] = 255;
        else
          cv[i] = 0;
        break;
      case 3:  //PLUSE 3
        if (wave_position[i] > lfo_length / 4)
          cv[i] = 255;
        else
          cv[i] = 0;
        break;
      case 4:  //SAW DOWN
        cv[i] = (1 - (wave_position[i] / lfo_length)) * 255;
        break;
      case 5:
      case 6:  //SAW UP 256=2^8
        cv[i] = (wave_position[i] / lfo_length) * 255;
        break;
      case 7:  //Random
        cv[i] = random(0, 255);
        break;
    }
  }

  //cv output
  for (int i = 0; i < PIN_NUM; i++) {
    int tmp_cv = cv[i];
    if (tmp_cv < 0) tmp_cv = 0;
    if (tmp_cv > 255) tmp_cv = 255;
    analogWrite(CV_PIN[i], tmp_cv);
  }

  Serial.print(" a1=");
  Serial.print((4096 - analogRead(KNOB_PIN)));
  Serial.print(" knob_coef=");
  Serial.print(knob_coef);
  Serial.print(" d7=");
  Serial.print(digitalRead(BUTTON_PIN));
  Serial.print(" lfo_type=");
  Serial.print(lfo_type);
  Serial.print(" p=");
  Serial.print(wave_position[0]);
  Serial.print(" cv=");
  Serial.print(cv[0]);
  for (float i = 0; i < cv[0] / 4; i++)
    Serial.print("|");

  Serial.print(" \n");
}

void genCoefValue() {
  coef[0] = 1;
  coef[1] = random(0, 300) / 1000 + 0.7;
  coef[2] = random(0, 200) / 1000 + 0.6;
  coef[3] = random(0, 200) / 1000 + 0.4;
  coef[4] = random(0, 200) / 1000 + 0.2;
  coef[5] = random(0, 200) / 1000 + 0.01;
  // float coef[PIN_NUM] = { 1, 0.73, 0.57, 0.41, 0.29, 0.13 };
}

void showKnobValue() {
  int tmp_knob_value = (4096 - analogRead(KNOB_PIN));
  int tmp_knob_value2 = map(tmp_knob_value, 0, 4096, 0, 6);
  Serial.print(" tmp_knob_value=");
  Serial.print(tmp_knob_value2);
  for (int i = 0; i < PIN_NUM; i++) {
    if (tmp_knob_value2 > i) {
      digitalWrite(CV_PIN[i], HIGH);
    } else {
      digitalWrite(CV_PIN[i], LOW);
    }
  }
}
