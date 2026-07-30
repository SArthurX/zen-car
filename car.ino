// ==========================================
// 自平衡車 - 純角度 PID 平衡主程式 (Arduino Mega 2560)
// ==========================================

#include <Wire.h>
#include "config.h"

// 外部模組函式宣告
void imu_init();
void imu_calibrate();
void imu_update();
void motors_init();
void set_motor_speed(int m1_speed, int m2_speed);

// ==========================================
// 全域變數
// ==========================================
// IMU 姿態 (由 imu.ino 更新)
float angleX = 0;
float gyro_rateX_current = 0;

// 角度環 PID 預設參數 (實測靜止平衡點約 23~24 度)
float Kp = 20.0;             // 比例增益
float Ki = 0.0;              // 積分增益
float Kd = 0.5;              // 微分增益 (阻尼係數)
float target_angle = 23.0;   // 平衡目標角度 (度)

float integral = 0.0;        // 積分累加值
int motor_output_debug = 0;  // 馬達 PWM 輸出值

// 控制時間與狀態
unsigned long last_loop_time = 0;
unsigned long last_print_time = 0;
bool robot_running = true;

// 函式宣告
void print_serial_menu();
void check_serial_command();
void print_debug_status();

// ==========================================
// setup
// ==========================================
void setup() {
  // 1. 初始化 USB Serial
  Serial.begin(SERIAL_BAUD);

  // 2. 初始化 Mega2560 藍芽 BT_SERIAL (Serial2: Pin 17 RX2 / Pin 16 TX2)
  BT_SERIAL.begin(SERIAL_BAUD);
  
  motors_init();
  imu_init();
  imu_calibrate();

  print_serial_menu();
  last_loop_time = millis();
}

// ==========================================
// main loop (200Hz 控制迴圈)
// ==========================================
void loop() {
  unsigned long now = millis();

  // 1. 處理通道指令
  check_serial_command();

  // 2. 定時控制迴圈 (每 5ms / 200Hz 執行一次)
  if (now - last_loop_time >= LOOP_TIME_MS) {
    last_loop_time = now;

    // update IMU
    imu_update();

    // 計算角度誤差
    float error = angleX - target_angle;

    // 安全檢查：若手動停止或傾角大於 SAFETY_ANGLE，切斷馬達
    if (!robot_running || abs(error) > SAFETY_ANGLE) {
      set_motor_speed(0, 0);
      integral = 0.0;
      motor_output_debug = 0;
    } 
    else {
      // PID 計算
      // P 項
      float P_out = Kp * error;

      // I 項 (抗飽和限制)
      float dt = LOOP_TIME_MS / 1000.0;
      integral += error * dt;
      integral = constrain(integral, -50.0, 50.0);
      float I_out = Ki * integral;

      // D 項 (使用陀螺儀角速度)
      float D_out = Kd * gyro_rateX_current;

      // 馬達輸出計算
      float pid_output = -(P_out + I_out + D_out);

      // 限制輸出最大 PWM
      int motor_cmd = constrain((int)pid_output, -PID_OUTPUT_MAX, PID_OUTPUT_MAX);
      motor_output_debug = motor_cmd;

      // 驅動雙輪馬達
      set_motor_speed(motor_cmd, motor_cmd);
    }
  }

  // 3. 定時發送即時 telemetry 姿態波形數據 (每 100ms / 10Hz)
  if (now - last_print_time >= PRINT_INTERVAL) {
    last_print_time = now;
    print_debug_status();
  }
}

// ==========================================
// 數據與波形格式輸出
// ==========================================
void print_debug_status() {
  float error = angleX - target_angle;
  
  String packet = "T:" + String(target_angle, 1) + 
                  ",A:" + String(angleX, 1) + 
                  ",M:" + String(motor_output_debug) + 
                  ",P:" + String(Kp, 1) + 
                  ",D:" + String(Kd, 2) + 
                  ",I:" + String(Ki, 2);

  if (abs(error) > SAFETY_ANGLE) packet += " [SAFE_CUT]";
  if (!robot_running) packet += " [STOPPED]";

  Serial.println(packet);
  BT_SERIAL.println(packet);
}