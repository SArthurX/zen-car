#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// 硬體腳位定義 (Arduino Mega 2560)
// ==========================================
#define MPU_ADDR 0x68

// L298P 馬達驅動腳位
#define M1_PWM_PIN 10
#define M1_DIR_PIN 12
#define M2_PWM_PIN 11
#define M2_DIR_PIN 13

// 編碼器腳位 (使用 Mega2560 外部中斷腳位 Pin 18/19 & Pin 2/3)
#define M1_ENCODER_A 19
#define M1_ENCODER_B 18
#define M2_ENCODER_A 2
#define M2_ENCODER_B 3

// ==========================================
// 藍牙串口定義 (使用 Serial2: Pin 17 RX2, Pin 16 TX2)
// ==========================================
#define BT_SERIAL Serial2
#define SERIAL_BAUD 9600

// ==========================================
// 控制迴圈與安全設定
// ==========================================
#define LOOP_TIME_MS    5     // 5ms = 200Hz 控制頻率
#define PRINT_INTERVAL  100   // 每 100ms 印出姿態狀態 (10Hz)
#define SAFETY_ANGLE    20.0  // 偏離目標角度超過此值即斷電保護 (度)

// ==========================================
// 馬達死區補償與輸出限制 (由實測數據獲得)
// ==========================================
#define DEAD_ZONE_L     23    // 左輪最低啟動 PWM 補償 (實測約 22~23)
#define DEAD_ZONE_R     22    // 右輪最低啟動 PWM 補償 (實測約 21~22)
#define MOTOR_DEAD_BAND 3     // PID 輸出絕對值低於此值停轉
#define PID_OUTPUT_MAX  240   // PID 輸出 PWM 上限 (最大 255)

// ==========================================
// MPU6050 互補濾波
// ==========================================
#define FILTER_GYRO     0.96  // 陀螺儀權重 (96% 陀螺儀 + 4% 加速度計)

#endif
