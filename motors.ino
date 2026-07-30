// ==========================================
// 馬達驅動模組 (L298P)
// ==========================================
#include <Arduino.h>
#include "config.h"

void drive_single_motor(int dir_pin, int pwm_pin, int speed, int dead_zone);

void motors_init() {
  pinMode(M1_PWM_PIN, OUTPUT);
  pinMode(M1_DIR_PIN, OUTPUT);
  pinMode(M2_PWM_PIN, OUTPUT);
  pinMode(M2_DIR_PIN, OUTPUT);
  
  analogWrite(M1_PWM_PIN, 0);
  analogWrite(M2_PWM_PIN, 0);
}

void set_motor_speed(int m1_speed, int m2_speed) {
  drive_single_motor(M1_DIR_PIN, M1_PWM_PIN, m1_speed, DEAD_ZONE_L);
  drive_single_motor(M2_DIR_PIN, M2_PWM_PIN, m2_speed, DEAD_ZONE_R);
}

void drive_single_motor(int dir_pin, int pwm_pin, int speed, int dead_zone) {
  // 小於死帶門檻直接停轉
  if (abs(speed) < MOTOR_DEAD_BAND) {
    analogWrite(pwm_pin, 0);
    return;
  }

  // 設定正反轉
  if (speed > 0) {
    digitalWrite(dir_pin, HIGH);
  } else {
    digitalWrite(dir_pin, LOW);
    speed = -speed;
  }

  // 加入啟動死區補償值
  int pwm = speed + dead_zone;
  pwm = constrain(pwm, 0, 255);
  analogWrite(pwm_pin, pwm);
}
